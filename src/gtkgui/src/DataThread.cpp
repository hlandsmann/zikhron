
#include <DataThread.h>
#include <annotation/CardDB.h>
#include <annotation/Ease.h>
#include <annotation/Markup.h>
#include <annotation/ZH_Annotator.h>
#include <dictionary/ZH_Dictionary.h>
#include <fmt/format.h>
#include <misc/Config.h>
#include <misc/Identifier.h>
#include <spaced_repetition/CardMeta.h>
#include <spaced_repetition/DataBase.h>
#include <spaced_repetition/ITreeWalker.h>
#include <spdlog/spdlog.h>
#include <utils/Property.h>

#include <algorithm>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iterator>
#include <memory>
#include <mutex>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <optional>
#include <ranges>
#include <stop_token>
#include <string>
#include <string_view>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

#include <sys/types.h>
namespace ranges = std::ranges;
namespace fs = std::filesystem;

template<>
struct fmt::formatter<fs::path>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }
    template<typename FormatContext>
    auto format(const fs::path& path, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(), "{}", path.string());
    }
};

namespace {

auto get_zikhron_cfg() -> std::shared_ptr<zikhron::Config>
{
    auto path_to_exe = fs::read_symlink("/proc/self/exe");
    auto zikhron_cfg_json = path_to_exe.parent_path() / "config.json";
    return std::make_shared<zikhron::Config>(path_to_exe.parent_path());
}

template<class JsonType, class TypeOut>
auto extract(const nlohmann::json& json, const TypeOut& current, std::string_view key) -> TypeOut
{
    auto valueIt = json.find(std::string(key));
    auto correctType = [](decltype(valueIt)& it) {
        if constexpr (std::is_same_v<JsonType, std::string>) {
            return it->is_string();
        }
        if constexpr (std::is_same_v<JsonType, int>) {
            return it->is_number_integer();
        }
    };
    if (valueIt != json.end() && correctType(valueIt)) {
        return JsonType(*valueIt);
    }
    spdlog::info("in main config: key \"{}\" was set to {}", key, current);
    return current;
}

} // namespace

void Session::ConfigMain::fromJson(const nlohmann::json& json)
{
    setDefault();
    lastVideoFile = extract<std::string, path>(json, lastVideoFile, s_last_video_file);
    lastAudioFile = extract<std::string, path>(json, lastAudioFile, s_last_audio_file);
    activePage = extract<int, int>(json, activePage, s_active_page);
}

auto Session::ConfigMain::toJson() const -> nlohmann::json
{
    nlohmann::json json;
    json[std::string(s_last_video_file)] = lastVideoFile;
    json[std::string(s_last_audio_file)] = lastAudioFile;
    json[std::string(s_active_page)] = activePage;
    return json;
}

Session::Session()
{
    open();
}

Session::~Session()
{
    if (save_config) {
        save();
    }
}

void Session::open()
{
    auto zikhron_config_file = zikhron_config_dir / config_file;
    if (fs::exists(zikhron_config_file)) {
        try {
            std::ifstream ifs(zikhron_config_file);
            cfgMain.fromJson(nlohmann::json::parse(ifs));
        } catch (const std::exception& e) {
            save_config = false; // if the user edits a config file and creates a syntax error, the file
                                 // will not be overwritten
            spdlog::error("Failed to parse zikhron config file: \"{}\"", e.what());
        }
    } else {
        spdlog::info("Zikhron config file will be created on exit!");
        cfgMain.setDefault();
    }
}

void Session::save()
{
    auto zikhron_config_file = zikhron_config_dir / config_file;
    auto json = cfgMain.toJson();

    fs::create_directories(zikhron_config_dir);
    std::ofstream ofs(zikhron_config_file);
    ofs << json.dump(4);
}

auto Session::ConfigDir() -> path
{
    std::string home = std::getenv("HOME");
    path config_dir = home.empty() ? path(home) / ".config" : "~/.config";
    return config_dir / "zikhron";
}

auto DataThread::get() -> DataThread&
{
    static std::unique_ptr<DataThread> dataThread;
    if (!dataThread) {
        dataThread = std::unique_ptr<DataThread>(new DataThread());
    }
    return *dataThread;
}

DataThread::DataThread()
{
    utl::PropertyServer::get().setSignalUpdate([dispatch = std::weak_ptr(propertyUpdate)]() {
        if (auto update = dispatch.lock(); update != nullptr) {
            update->emit();
        }
    });
    job_queue.emplace([this]() {
        config = get_zikhron_cfg();
        db = std::make_shared<sr::DataBase>(config);
        treeWalker = sr::ITreeWalker::createTreeWalker(db);
    });

    dispatcher.connect([this]() { dispatcher_fun(); });
    propertyUpdate->connect([]() { utl::PropertyServer::get().updateProperties(); });
    worker = std::jthread([this](std::stop_token token) { worker_thread(std::move(token)); });
}

DataThread::~DataThread()
{
    worker.request_stop();
    worker.join();
    zikhronCfg.save();
    saveProgress();
}

void DataThread::saveProgress()
{
    db->saveProgress();
}

void DataThread::worker_thread(std::stop_token token)
{
    std::unique_lock lock(condition_mutex);

    while (!token.stop_requested()) {
        while (not job_queue.empty()) {
            job_queue.front()();
            job_queue.pop();
        }
        dispatcher.emit();

        condition.wait(lock, token, [this, &token]() {
            if (token.stop_requested()) {
                return false;
            }
            return not job_queue.empty();
        });
    }

    spdlog::info("DataThread is exiting..");
}

void DataThread::dispatcher_fun()
{
    std::lock_guard<std::mutex> lock(condition_mutex);
    while (not dispatch_queue.empty()) {
        dispatch_queue.front()();
        dispatch_queue.pop();
    }
}

void DataThread::requestCard(std::optional<CardId> preferedCardId)
{
    {
        std::lock_guard<std::mutex> lock(condition_mutex);
        job_queue.emplace([this, preferedCardId]() {
            auto& cardMeta = treeWalker->getNextCardChoice(preferedCardId);
            sendActiveCard(cardMeta);
        });
    }
    condition.notify_one();
}

void DataThread::requestCardFromIds(std::vector<uint>&& _ids)
{
    {
        std::lock_guard<std::mutex> lock(condition_mutex);
        job_queue.emplace([this, ids = std::move(_ids)]() {
            std::vector<paragraph_optional> paragraphs;
            ranges::transform(
                    ids, std::back_inserter(paragraphs), [this](auto id) { return getCardFromId(id); });
            dispatch_queue.emplace([this, paragraphs = std::move(paragraphs)]() mutable {
                if (send_card) {
                    send_paragraphFromIds(std::move(paragraphs));
                }
            });
        });
    }
    condition.notify_one();
}

void DataThread::submitEase(const sr::ITreeWalker::Id_Ease_vt& ease)
{
    std::lock_guard<std::mutex> lock(condition_mutex);
    treeWalker->setEaseLastCard(ease);
}

void DataThread::signal_annotation_connect(const signal_annotation& signal)
{
    std::lock_guard<std::mutex> lock(condition_mutex);
    send_annotation = signal;
}

void DataThread::signal_card_connect(const signal_card& signal)
{
    std::lock_guard<std::mutex> lock(condition_mutex);
    send_card = signal;
}

void DataThread::signal_paragraphFromIds_connect(const signal_paragraphFromIds& signal)
{
    std::lock_guard<std::mutex> lock(condition_mutex);
    send_paragraphFromIds = signal;
}

void DataThread::dispatch_arbitrary(const std::function<void()>& fun)
{
    std::lock_guard<std::mutex> lock(condition_mutex);
    dispatch_queue.push(fun);
    dispatcher.emit();
}

auto DataThread::getCardFromId(uint id) const -> std::optional<std::shared_ptr<markup::Paragraph>>
{
    // TODO reactivate
    //  auto opt_cardInformation = vocabularySR->getCardFromId(id);
    //  if (!opt_cardInformation.has_value()) {
    //      return {};
    //  }
    //  auto [current_card, vocableIds, ease] = std::move(opt_cardInformation.value());
    //  auto paragraph = std::make_unique<markup::Paragraph>(std::move(current_card), std::move(vocableIds));
    //  paragraph->setupVocables(ease);
    //
    //  return {std::move(paragraph)};
    return {};
}

void DataThread::sendActiveCard(sr::CardMeta& cardMeta)
{
    message_card msg_card = std::make_shared<sr::CardMeta>(cardMeta);
    message_annotation paragraph_annotation = cardMeta.getAnnotationMarkup();
    // auto [current_card, vocableIds, ease] = std::move(cardInformation);
    // if (not current_card.has_value()) {
    //     return;
    // }
    // // TODO remove static_cast CardId
    // CardId cardId = current_card.value()->Id();
    // auto current_card_clone = std::unique_ptr<Card>(current_card.value()->clone());
    // auto paragraph = std::make_unique<markup::Paragraph>(std::move(current_card.value()), std::move(vocableIds));
    // auto paragraph_annotation = std::make_shared<markup::Paragraph>(std::move(current_card_clone));
    // paragraph->setupVocables(ease);
    // auto orderedEase = paragraph->getRelativeOrderedEaseList(ease);
    // std::vector<Ease> vocEaseList;
    // ranges::copy(orderedEase | std::views::values, std::back_inserter(vocEaseList));

    dispatch_queue.emplace(
            [this, msg_card = std::move(msg_card)]() mutable {
                if (send_card) {
                    send_card(msg_card);
                }
            });
    dispatch_queue.emplace([this, msg_annotation = std::move(paragraph_annotation)]() mutable {
        if (send_annotation != nullptr) {
            send_annotation(msg_annotation);
        }
    });
}

void DataThread::submitAnnotation(const ZH_Annotator::Combination& combination,
                                  const ZH_Annotator::CharacterSequence& characterSequence)
{
    {
        std::lock_guard<std::mutex> lock(condition_mutex);
        job_queue.emplace([this, combination, characterSequence]() {
            // auto cardInformation = vocabularySR->AddAnnotation(combination, characterSequence);
            // TODO reactivate
            //  sendActiveCard(cardInformation);
        });
    }
    condition.notify_one();
}

void DataThread::submitVocableChoice(uint vocId, uint vocIdOldChoice, uint vocIdNewChoice)
{
    {
        std::lock_guard<std::mutex> lock(condition_mutex);
        job_queue.emplace([this, vocId, vocIdOldChoice, vocIdNewChoice]() {
            // auto cardInformation = vocabularySR->AddVocableChoice(vocId, vocIdOldChoice, vocIdNewChoice);
            // TODO reactivate
            //  sendActiveCard(cardInformation);
        });
    }
    condition.notify_one();
}
