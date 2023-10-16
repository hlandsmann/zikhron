#pragma once
#include <annotation/Card.h>
#include <annotation/Ease.h>
#include <annotation/Markup.h>
#include <annotation/ZH_Annotator.h>
#include <dictionary/ZH_Dictionary.h>
#include <glibmm/dispatcher.h>
#include <gtkmm.h>
#include <misc/Config.h>
#include <misc/Identifier.h>
#include <spaced_repetition/ITreeWalker.h>
#include <spaced_repetition/Vocabulary.h>
#include <utils/Property.h>

#include <queue>

#include <condition_variable>
#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <nlohmann/json_fwd.hpp>
#include <optional>
#include <stop_token>
#include <string_view>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>

#include <sys/types.h>

class Session
{
    using path = std::filesystem::path;

public:
    struct ConfigMain
    {
        static constexpr std::string_view s_last_video_file = "last_video_file";
        static constexpr std::string_view s_last_audio_file = "last_audio_file";
        static constexpr std::string_view s_active_page = "active_page";
        path lastVideoFile;
        path lastAudioFile;
        int activePage{};

    private:
        friend class Session;
        void setDefault()
        {
            lastVideoFile = "";
            activePage = 0;
        }
        void fromJson(const nlohmann::json& json);
        [[nodiscard]] auto toJson() const -> nlohmann::json;
    };
    ConfigMain cfgMain;

private:
    friend class DataThread;
    Session();
    ~Session();
    [[nodiscard]] static auto ConfigDir() -> path;
    void open();
    void save();

    path zikhron_config_dir = ConfigDir();
    path config_file = "session.json";

    bool save_config = true;
};

class DataThread
{
    DataThread();

public:
    static auto get() -> DataThread&;
    ~DataThread();
    using paragraph_optional = std::optional<std::shared_ptr<markup::Paragraph>>;
    using message_card = std::tuple<std::shared_ptr<markup::Paragraph>, std::vector<Ease>, CardId>;
    using message_annotation = std::shared_ptr<markup::Paragraph>;
    using message_paragraphFromIds = std::vector<paragraph_optional>;
    using signal_card = std::function<void(message_card&)>;
    using signal_annotation = std::function<void(message_annotation&)>;
    using signal_paragraphFromIds = std::function<void(message_paragraphFromIds&&)>;
    Session zikhronCfg;
    void saveProgress();

    void requestCard(std::optional<CardId> preferedCardId = {});
    void requestCardFromIds(std::vector<uint>&& ids);
    void submitEase(const VocabularySR::Id_Ease_vt& ease);
    void submitAnnotation(const ZH_Annotator::Combination& combination,
                          const ZH_Annotator::CharacterSequence& characterSequence);
    void submitVocableChoice(uint vocId, uint vocIdOldChoice, uint vocIdNewChoice);

    void signal_annotation_connect(const signal_annotation& signal);
    void signal_card_connect(const signal_card& signal);
    void signal_paragraphFromIds_connect(const signal_paragraphFromIds& signal);
    void dispatch_arbitrary(const std::function<void()>& fun);

private:
    static constexpr std::string_view path_to_dictionary =
            "/home/harmen/src/zikhron/dictionaries/cedict_ts.u8";
    static constexpr std::string_view path_to_cardDB = "/home/harmen/zikhron/cards";

    using Item_Id_vt = std::vector<std::pair<ZH_Dictionary::Entry, VocableId>>;
    using Id_Ease_vt = std::map<VocableId, Ease>;
    using CardInformation = sr::ITreeWalker::CardInformation;

    void worker_thread(std::stop_token);
    void dispatcher_fun();
    void sendActiveCard(CardInformation& cardInformation);

    [[nodiscard]] auto getCardFromId(uint id) const -> paragraph_optional;
    std::shared_ptr<zikhron::Config> config;
    std::shared_ptr<sr::ITreeWalker> treeWalker;

    std::jthread worker;
    std::condition_variable_any condition;
    std::mutex condition_mutex;

    std::shared_ptr<CardDB> cardDB;

    signal_card send_card;
    signal_annotation send_annotation;
    signal_paragraphFromIds send_paragraphFromIds;

    // message_card msg_card;
    // message_annotation msg_annotation;

    std::shared_ptr<ZH_Dictionary> zh_dictionary;
    std::unique_ptr<VocabularySR> vocabularySR;
    Glib::Dispatcher dispatcher;
    std::shared_ptr<Glib::Dispatcher> propertyUpdate = std::make_shared<Glib::Dispatcher>();

    std::queue<std::function<void()>> job_queue;
    std::queue<std::function<void()>> dispatch_queue;
};
