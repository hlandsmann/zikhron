#include "VocabularySR.h"
#include <TextCard.h>
#include <utils/Markup.h>
#include <utils/StringU8.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <ranges>

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
    std::sort(cardMeta.begin(), cardMeta.end(), [](const auto& a, const auto& b) {
        return a.value > b.value;
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
    std::vector<ZH_Dictionary::Item> relevantVocables;
    auto cardMetaIt = ranges::find_if(cardMeta, [&](const CardMeta& cm) { return cm.cardId == cardId; });
    ranges::transform(cardMetaIt->vocableIds | std::views::filter([&](uint vocId) {
                          return id_vocableSR.find(vocId) == id_vocableSR.end();
                      }),
                      std::back_inserter(relevantVocables),
                      [&](uint vocId) -> ZH_Dictionary::Item { return id_vocable.at(vocId).front(); });

    std::cout << "CardID: " << cardId;
    std::cout << "size one: " << relevantVocables.size()
              << " size two: " << cardMeta.at(cardId).vocableIds.size() << "\n";
    return relevantVocables;
}
