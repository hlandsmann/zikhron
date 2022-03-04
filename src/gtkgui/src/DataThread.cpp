#include <DataThread.h>
#include <annotation/TextCard.h>
#include <dictionary/ZH_Dictionary.h>
#include <spdlog/spdlog.h>
#include <algorithm>
#include <ranges>
namespace ranges = std::ranges;

namespace {

auto loadCardDB(const std::string& path_to_cardDB) -> CardDB {
    CardDB cardDB;
    try {
        cardDB.loadFromDirectory(path_to_cardDB);
    }
    catch (const std::exception& e) {
        spdlog::error(e.what());
    }
    catch (...) {
        spdlog::error("Unknown Error, load Card Database failed!");
    }
    return cardDB;
}

}  // namespace

DataThread::DataThread(const signal_card& receive_paragraph) : send_card(receive_paragraph) {
    current_job = [this]() {
        CardDB cardDB = loadCardDB(std::string{path_to_cardDB});
        zh_dictionary = std::make_shared<ZH_Dictionary>(path_to_dictionary);
        vocabularySR = std::make_unique<VocabularySR>(std::move(cardDB), zh_dictionary);
        auto cardInformation = vocabularySR->getCard();
        sendActiveCard(std::move(cardInformation));
    };

    dispatcher.connect([this]() { dispatcher_fun(); });
    worker = std::jthread([this](std::stop_token token) { worker_thread(token); });
}
DataThread::~DataThread() {
    worker.request_stop();
    worker.join();
}

void DataThread::worker_thread(std::stop_token token) {
    std::unique_lock lock(condition_mutex);

    do {
        if (current_job) {
            current_job();
            dispatcher.emit();
        }
        current_job = {};
        condition.wait(lock, token, [this, &token]() {
            if (token.stop_requested())
                return false;
            return static_cast<bool>(current_job);
        });
    } while (!token.stop_requested());

    spdlog::info("DataThread is exiting..");
}

void DataThread::dispatcher_fun() {
    spdlog::info("Dispatch..");
    if (send_card && msg_paragraph.first != nullptr) {
        send_card(std::move(msg_paragraph));
    }
}

void DataThread::requestCard() {
    {
        std::lock_guard<std::mutex> lock(condition_mutex);
        spdlog::info("Request card..");
        current_job = [this]() {
            auto cardInformation = vocabularySR->getCard();
            sendActiveCard(std::move(cardInformation));
        };
    }
    condition.notify_one();
}
void DataThread::submitEase(const VocabularySR::Id_Ease_vt& ease) {
    std::lock_guard<std::mutex> lock(condition_mutex);
    vocabularySR->setEaseLastCard(ease);
}

void DataThread::sendActiveCard(CardInformation&& cardInformation) {
    auto [current_card, vocables, ease] = std::move(cardInformation);
    auto current_card_clone = std::unique_ptr<Card>(current_card->clone());
    auto paragraph = std::make_unique<markup::Paragraph>(std::move(current_card));
    auto paragraph_annotation = std::make_unique<markup::Paragraph>(std::move(current_card_clone));
    paragraph->setupVocables(std::move(vocables));
    auto orderedEase = paragraph->getRelativeOrderedEaseList(ease);
    std::vector<Ease> vocEaseList;
    ranges::copy(orderedEase | std::views::values, std::back_inserter(vocEaseList));
    msg_paragraph = message_card(std::move(paragraph), std::move(vocEaseList));
}
