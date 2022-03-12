#include <DataThread.h>
#include <annotation/TextCard.h>
#include <dictionary/ZH_Dictionary.h>
#include <spdlog/spdlog.h>
#include <algorithm>
#include <ranges>
namespace ranges = std::ranges;

namespace {

auto loadCardDB(const std::string& path_to_cardDB) -> std::unique_ptr<CardDB> {
    auto cardDB = std::make_unique<CardDB>();
    try {
        cardDB->loadFromDirectory(path_to_cardDB);
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

std::unique_ptr<DataThread> dataThread;

auto DataThread::get() -> DataThread& {
    if (!dataThread)
        dataThread = std::unique_ptr<DataThread>(new DataThread());
    return *dataThread;
}

void DataThread::destroy() { dataThread = nullptr; }

DataThread::DataThread() {
    job_queue.push([this]() {
        std::shared_ptr<CardDB> cardDB = std::move(loadCardDB(std::string{path_to_cardDB}));
        zh_dictionary = std::make_shared<ZH_Dictionary>(path_to_dictionary);
        vocabularySR = std::make_unique<VocabularySR>(cardDB, zh_dictionary);
    });

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
        while (not job_queue.empty()) {
            job_queue.front()();
            job_queue.pop();
        }
        dispatcher.emit();

        condition.wait(lock, token, [this, &token]() {
            if (token.stop_requested())
                return false;
            return not job_queue.empty();
        });
    } while (!token.stop_requested());

    spdlog::info("DataThread is exiting..");
}

void DataThread::dispatcher_fun() {
    std::lock_guard<std::mutex> lock(condition_mutex);
    while (not dispatch_queue.empty()) {
        dispatch_queue.front()();
        dispatch_queue.pop();
    }
}

void DataThread::requestCard() {
    {
        std::lock_guard<std::mutex> lock(condition_mutex);
        job_queue.push([this]() {
            auto cardInformation = vocabularySR->getCard();
            sendActiveCard(cardInformation);
        });
    }
    condition.notify_one();
}
void DataThread::submitEase(const VocabularySR::Id_Ease_vt& ease) {
    std::lock_guard<std::mutex> lock(condition_mutex);
    vocabularySR->setEaseLastCard(ease);
}

void DataThread::signal_annotation_connect(const signal_annotation& signal) {
    std::lock_guard<std::mutex> lock(condition_mutex);
    send_annotation = signal;
}

void DataThread::signal_card_connect(const signal_card& signal) {
    std::lock_guard<std::mutex> lock(condition_mutex);
    send_card = signal;
}

void DataThread::sendActiveCard(CardInformation& cardInformation) {
    auto [current_card, vocableIds, ease] = std::move(cardInformation);
    auto current_card_clone = std::unique_ptr<Card>(current_card->clone());
    auto paragraph = std::make_unique<markup::Paragraph>(std::move(current_card), std::move(vocableIds));
    auto paragraph_annotation = std::make_shared<markup::Paragraph>(std::move(current_card_clone));
    paragraph->setupVocables(ease);
    auto orderedEase = paragraph->getRelativeOrderedEaseList(ease);
    std::vector<Ease> vocEaseList;
    ranges::copy(orderedEase | std::views::values, std::back_inserter(vocEaseList));

    dispatch_queue.push(
        [this, msg_card = message_card(std::move(paragraph), std::move(vocEaseList))]() mutable {
            if (send_card)
                send_card(msg_card);
        });
    dispatch_queue.push([this, msg_annotation = std::move(paragraph_annotation)]() mutable {
        if (send_annotation != nullptr) {
            send_annotation(msg_annotation);
        }
    });
}

void DataThread::submitAnnotation(const ZH_Annotator::Combination& combination,
                                  const ZH_Annotator::CharacterSequence& characterSequence) {
    {
        std::lock_guard<std::mutex> lock(condition_mutex);
        job_queue.push([this, combination, characterSequence]() {
            auto cardInformation = vocabularySR->AddAnnotation(combination, characterSequence);
            sendActiveCard(cardInformation);
        });
    }
    condition.notify_one();
}

void DataThread::submitVocableChoice(uint vocId, uint vocIdOldChoice, uint vocIdNewChoice) {
    {
        std::lock_guard<std::mutex> lock(condition_mutex);
        job_queue.push([this, vocId, vocIdOldChoice, vocIdNewChoice]() {
            auto cardInformation = vocabularySR->AddVocableChoice(vocId, vocIdOldChoice, vocIdNewChoice);
            sendActiveCard(cardInformation);
        });
    }
    condition.notify_one();
}
