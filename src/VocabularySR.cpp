#include "VocabularySR.h"
#include <TextCard.h>
#include <utils/Markup.h>
#include <utils/StringU8.h>
#include <algorithm>
#include <boost/intrusive/set.hpp>
#include <iostream>
VocabularySR::~VocabularySR() {}
VocabularySR::VocabularySR(CardDB&& _cardDB, std::shared_ptr<ZH_Dictionary> _zh_dictionary)
    : cardDB(std::make_shared<CardDB>(std::move(_cardDB))), zh_dictionary(_zh_dictionary) {
    std::set<ZH_Annotator::Item> myDic;

    GenerateFromCards();
}

void VocabularySR::GenerateFromCards() {
    const std::vector<CardDB::CardPtr>& cards = cardDB->get();
    for (const auto& card : cards) {
        utl::StringU8 card_text = markup::Paragraph::textFromCard(*card);
        ZH_Annotator annotator = ZH_Annotator(card_text, zh_dictionary);

        InsertVocabulary(annotator.UniqueItems(), card->id);
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
}

void VocabularySR::InsertVocabulary(const std::set<ZH_Annotator::Item>& cardVocabulary, int cardId) {
    for (auto& item : cardVocabulary) {
        if (auto it = vocables.find(item.dicItemVec); it != vocables.end()) {
            auto& vocable = it->second;
            vocable.cardIds.insert(cardId);
        } else {
            int nextFreeId = GetNextFreeId();
            vocables[item.dicItemVec] = {.id = nextFreeId, .cardIds = {cardId}};
            id_vocable[nextFreeId] = ZH_dicItemVec{item.dicItemVec};
        }
    }
}

auto VocabularySR::GetNextFreeId() -> int {
    if (not id_vocable.empty())
        return id_vocable.rbegin()->first + 1;
    else
        return 0;
}
