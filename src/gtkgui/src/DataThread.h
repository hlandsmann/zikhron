#pragma once

#include <annotation/Ease.h>
#include <annotation/Markup.h>
#include <dictionary/ZH_Dictionary.h>
#include <gtkmm.h>
#include <spaced_repetition/VocabularySR.h>
#include <condition_variable>
#include <functional>
#include <memory>
#include <queue>
#include <string_view>
#include <thread>
#include <tuple>

class DataThread {
    DataThread();

public:
    static auto get() -> DataThread&;
    static void destroy();
    ~DataThread();

    using message_card = std::pair<std::shared_ptr<markup::Paragraph>, std::vector<Ease>>;
    using message_annotation = std::shared_ptr<markup::Paragraph>;
    using signal_card = std::function<void(message_card&)>;
    using signal_annotation = std::function<void(message_annotation&)>;

    void requestCard();
    void submitEase(const VocabularySR::Id_Ease_vt& ease);
    void submitAnnotation(const ZH_Annotator::Combination& combination,
                          const ZH_Annotator::CharacterSequence& characterSequence);

    void signal_annotation_connect(const signal_annotation& signal);
    void signal_card_connect(const signal_card& signal);

private:
    static constexpr std::string_view path_to_dictionary =
        "/home/harmen/src/zikhron/dictionaries/cedict_ts.u8";
    static constexpr std::string_view path_to_cardDB = "/home/harmen/src/zikhron/conversion/xxcards";

    using Item_Id_vt = std::vector<std::pair<ZH_Dictionary::Entry, uint>>;
    using Id_Ease_vt = std::map<uint, Ease>;
    using CardInformation = std::tuple<std::unique_ptr<Card>, Item_Id_vt, Id_Ease_vt>;

    void worker_thread(std::stop_token);
    void dispatcher_fun();
    void sendActiveCard(CardInformation& cardInformation);

    std::jthread worker;
    std::condition_variable_any condition;
    std::mutex condition_mutex;

    signal_card send_card;
    signal_annotation send_annotation;
    // message_card msg_card;
    // message_annotation msg_annotation;

    std::shared_ptr<ZH_Dictionary> zh_dictionary;
    std::unique_ptr<VocabularySR> vocabularySR;
    Glib::Dispatcher dispatcher;

    std::queue<std::function<void()>> job_queue;
    std::queue<std::function<void()>> dispatch_queue;
};
