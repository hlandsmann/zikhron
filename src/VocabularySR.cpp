#include "VocabularySR.h"
#include <TextCard.h>
#include <utils/Markup.h>
#include <utils/StringU8.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <ranges>
#include "rapidjosnWrapper.h"

namespace ranges = std::ranges;

VocabularySR::~VocabularySR() {}
VocabularySR::VocabularySR(CardDB&& _cardDB, std::shared_ptr<ZH_Dictionary> _zh_dictionary)
    : cardDB(std::make_shared<CardDB>(std::move(_cardDB))), zh_dictionary(_zh_dictionary) {
    std::set<ZH_Annotator::Item> myDic;

    GenerateFromCards();
}

void VocabularySR::GenerateFromCards() {
    const std::map<uint, CardDB::CardPtr>& cards = cardDB->get();
    for (const auto& [cardId, card] : cards) {
        utl::StringU8 card_text = markup::Paragraph::textFromCard(*card);
        ZH_Annotator annotator = ZH_Annotator(card_text, zh_dictionary);

        InsertVocabulary(annotator.UniqueItems(), cardId);
    }
    std::cout << "Size: " << vocables.size();
    for (const auto& [voc, vocmeta] : vocables) {
        std::cout << voc.front().key << " : ";
        std::cout << vocmeta.id << " - ";
        for (const auto& num : vocmeta.cardIds)
            std::cout << num << ", ";
        std::cout << "\n";
    }

    for (const auto& voc : vocables) {
        if (voc.first.empty())
            continue;

        const auto word = utl::StringU8(voc.first.front().key);
        allCharacters.insert(word.cbegin(), word.cend());
    }
    for (const auto& mychar : allCharacters) {
        std::cout << mychar;
    }
    std::cout << "\n";
    std::cout << "Count of Characters: " << allCharacters.size() << "\n";
    std::cout << "VocableSize: " << vocables.size() << "\n";

    CalculateCardValue();
    for (const auto& cm : std::span(cardMeta.begin(), std::min(cardMeta.begin() + 32, cardMeta.end()))) {
        std::cout << "Cm id: " << cm.cardId << " value: " << cm.value << " len: " << cm.vocableIds.size()
                  << "\n";
        for (uint vid : cm.vocableIds) {
            std::cout << " - " << id_vocable[vid].front().key;
        }
        std::cout << "\n";
    }
}

void VocabularySR::CalculateCardValue() {
    for (CardMeta& cm : cardMeta) {
        float quantity_of_vocable_usage = 0;
        for (auto vId : cm.vocableIds) {
            const auto& vocable = id_vocable[vId];
            const auto& vocableMeta = vocables[vocable];
            quantity_of_vocable_usage += vocableMeta.cardIds.size();
        }
        cm.value = quantity_of_vocable_usage / std::sqrt(static_cast<float>(cm.vocableIds.size()));
    }

    ranges::sort(cardMeta, std::ranges::greater{}, &CardMeta::value);
    ranges::transform(cardMeta,
                      std::inserter(id_cardMeta, id_cardMeta.begin()),
                      [](const CardMeta& cm) -> std::pair<uint, CardMeta> {
                          return {cm.cardId, cm};
                      });
}

void VocabularySR::InsertVocabulary(const std::set<ZH_Annotator::Item>& cardVocabulary, uint cardId) {
    if (cardMeta.size() <= cardId)
        cardMeta.resize(cardId + 1);

    for (auto& item : cardVocabulary) {
        if (auto it = vocables.find(item.dicItemVec); it != vocables.end()) {
            auto& vocable = it->second;
            vocable.cardIds.insert(cardId);

            cardMeta[cardId].vocableIds.insert(it->second.id);
        } else {
            uint nextFreeId = GetNextFreeId();
            vocables[item.dicItemVec] = {.id = nextFreeId, .cardIds = {cardId}};
            id_vocable[nextFreeId] = ZH_dicItemVec{item.dicItemVec};

            cardMeta[cardId].vocableIds.insert(nextFreeId);
        }
    }
    cardMeta[cardId].cardId = cardId;
}

auto VocabularySR::GetNextFreeId() -> uint {
    if (not id_vocable.empty())
        return id_vocable.rbegin()->first + 1;
    else
        return 0;
}

auto VocabularySR::getCard() -> std::pair<std::unique_ptr<Card>, std::vector<ZH_Dictionary::Item>> {
    if (cardMeta.empty())
        return {nullptr, {}};
    uint cardId = cardMeta.front().cardId;

    return {std::unique_ptr<Card>(cardDB->get().at(cardId)->clone()), GetRelevantVocables(cardId)};
}

auto VocabularySR::GetRelevantVocables(uint cardId) -> std::vector<ZH_Dictionary::Item> {
    std::set<uint> activeVocables;
    const CardMeta& cm = id_cardMeta.at(cardId);

    auto vocableActive = [&](uint vocId) -> bool {
        return id_vocableSR.find(vocId) == id_vocableSR.end();
    };
    ranges::copy(cm.vocableIds | std::views::filter(vocableActive),
                 std::inserter(activeVocables, activeVocables.begin()));

    std::vector<ZH_Dictionary::Item> relevantVocables;
    ranges::transform(activeVocables,
                      std::back_inserter(relevantVocables),
                      [&](uint vocId) -> ZH_Dictionary::Item { return id_vocable.at(vocId).front(); });

    std::cout << "CardID: " << cardId;
    std::cout << "size one: " << relevantVocables.size()
              << " size two: " << cardMeta.at(cardId).vocableIds.size() << "\n";

    ids_activeVocables = std::move(activeVocables);
    return relevantVocables;
}

void VocabularySR::setEaseLastCard(Ease ease) {
    using namespace std::literals;
    float easeChange = [ease]() -> float {
        switch (ease) {
        case Ease::easy: return 1.2;
        case Ease::good: return 1.0;
        case Ease::hard: return 0.8;
        default: return 0.;
        }
    }();

    for (uint id : ids_activeVocables) {
        auto& vocableSR = id_vocableSR[id];
        vocableSR.ease_factor = std::clamp(vocableSR.ease_factor * easeChange, 1.3f, 2.5f);
        vocableSR.last_seen = std::chrono::steady_clock::now();

        vocableSR.advanceByEase(ease);
    }
    SaveProgress();
}

void VocabularySR::SaveProgress() {
    // rapidjson::Document jsonVocSR;
    // rapidjson::Document::AllocatorType& allocator = jsonVocSR.GetAllocator();
    // rapidjson::Value content(rapidjson::kArrayType);

    // for (const auto& [id, vocSR] : id_vocableSR) {
    //     rapidjson::Value v;
    //     v["a"] = "b";
    // }

    // rapidjson::StringBuffer buffer;
    // rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    // jsonVocSR.Accept(writer);
    // std::cout << buffer.GetString() << "\n";
}

void VocableSR::advanceByEase(Ease ease) {
    if (ease == Ease::again) {
        interval_day = std::chrono::minutes{10};
    } else {
        interval_day = std::max(day_t{1}, interval_day * ease_factor);
    }
}
