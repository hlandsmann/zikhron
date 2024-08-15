#include <CardMeta.h>
#include <DataBase.h>
#include <VocableMeta.h>
#include <annotation/Ease.h>
#include <database/CbdFwd.h>
#include <database/TokenizationChoiceDB.h>
#include <database/VocableProgress.h>
#include <database/WordDB.h>
#include <dictionary/ZH_Dictionary.h>
#include <fmt/format.h>
#include <misc/Config.h>
#include <misc/Identifier.h>
#include <spdlog/spdlog.h>
#include <srtypes.h>
#include <utils/StringU8.h>
#include <utils/format.h>
#include <utils/index_map.h>
#include <utils/min_element_val.h>

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <map>
#include <memory>
#include <ranges>
#include <set>
#include <stdexcept>
#include <utility>

#include <sys/types.h>

namespace ranges = std::ranges;
namespace views = std::views;

namespace sr {
DataBase::DataBase(std::shared_ptr<zikhron::Config> _config,
                   std::shared_ptr<WordDB> _wordDB,
                   std::shared_ptr<CardDB> _cardDB,
                   std::shared_ptr<TokenizationChoiceDB> _tokenizationChoiceDB)
    : config{std::move(_config)}
    , wordDB{std::move(_wordDB)}
    , cardDB{std::move(_cardDB)}
    , tokenizationChoiceDB{std::move(_tokenizationChoiceDB)}
    , vocables{std::make_shared<utl::index_map<VocableId, VocableMeta>>()}
{
    fillIndexMaps();
}

DataBase::~DataBase()
{
    save();
}

void DataBase::save()
{
    wordDB->save();
    tokenizationChoiceDB->save();
    cardDB->save();
}

auto DataBase::Vocables() const -> const utl::index_map<VocableId, VocableMeta>&
{
    return *vocables;
}

auto DataBase::MetaCards() -> const utl::index_map<CardId, CardMeta>&
{
    return metaCards;
}

auto DataBase::getTokenizationChoiceDB() const -> std::shared_ptr<database::TokenizationChoiceDB>
{
    return tokenizationChoiceDB;
}

auto DataBase::getCardDB() const -> std::shared_ptr<CardDB>
{
    return cardDB;
}

auto DataBase::getWordDB() const -> std::shared_ptr<WordDB>
{
    return wordDB;
}

auto DataBase::getCardMeta(const database::CardPtr& card) -> const CardMeta&
{
    auto cardId = card->getCardId();
    if (metaCards.contains(cardId)) {
        return metaCards.at_id(cardId).second;
    }
    temporaryCardMeta = std::make_shared<CardMeta>(cardId, card, vocables);
    for (VocableId vocId : temporaryCardMeta->VocableIds()) {
        if (vocables->contains(vocId)) {
            continue;
        }

        const auto& word = wordDB->lookupId(vocId);
        vocables->emplace(word->getId(), word->getProgress());
    }
    return *temporaryCardMeta;
}

auto DataBase::getCardMeta(CardId cardId) -> const CardMeta&
{
    if (metaCards.contains(cardId)) {
        return metaCards.at_id(cardId).second;
    }
    if (temporaryCardMeta && temporaryCardMeta->Id() == cardId) {
        return *temporaryCardMeta;
    }
    throw std::runtime_error(fmt::format("Bad Card Id {}", cardId));
}

void DataBase::reloadCard(const database::CardPtr& card)
{
    const auto& [cardIndex, cardMeta] = metaCards.at_id(card->getCardId());
    for (auto vocableIndex : cardMeta.VocableIndices()) {
        auto& vocableMeta = (*vocables)[vocableIndex];
        vocableMeta.cardIndices_erase(cardIndex);
    }
    auto tokenizationChoices = tokenizationChoiceDB->getChoicesForCard(card->getCardId());
    card->setTokenizationChoices(tokenizationChoices);
    metaCards[cardIndex].resetMetaData();
    for (VocableId vocId : cardMeta.VocableIds()) {
        if (vocables->contains(vocId)) {
            continue;
        }
        const auto& word = wordDB->lookupId(vocId);
        vocables->emplace(word->getId(), word->getProgress());
    }
    for (auto vocableIndex : cardMeta.VocableIndices()) {
        auto& vocableMeta = (*vocables)[vocableIndex];
        vocableMeta.cardIndices_insert(cardIndex);
    }
}

void DataBase::setEaseVocable(VocableId vocId, const Ease& ease)
{
    VocableMeta& vocable = vocables->at_id(vocId).second;
    vocable.advanceByEase(ease);
}

void DataBase::triggerVocable(VocableId vocId, CardId cardId)
{
    VocableMeta& vocable = vocables->at_id(vocId).second;
    vocable.triggerByCardId(cardId, metaCards);
}

void DataBase::resetCardsContainingVocable(VocableId vocId)
{
    const auto& [_, vocable] = vocables->at_id(vocId);
    const auto& cardIndices = vocable.CardIndices();
    for (size_t card_index : cardIndices) {
        auto& card = metaCards[card_index];
        card.resetTimingAndVocables();
    }
}

auto DataBase::generateVocableIdProgressMap() const -> std::map<VocableId, VocableProgress>
{
    std::map<VocableId, VocableProgress> id_progress;
    ranges::transform(vocables->id_index_view(),
                      std::inserter(id_progress, id_progress.begin()),
                      [this](const auto& id_index) -> std::pair<VocableId, VocableProgress> {
                          const auto [vocableId, index] = id_index;
                          return {vocableId, (*vocables)[index].Progress()};
                      });
    return id_progress;
}

void DataBase::fillIndexMaps()
{
    vocId_set allVocableIds;
    const std::map<CardId, database::CardPtr>& cards = cardDB->getCards();
    for (const auto& [cardId, choices] : tokenizationChoiceDB->getChoicesForCards()) {
        const auto& cardPtr = cards.at(cardId);
        cardPtr->setTokenizationChoices(choices);
    }
    for (const auto& [id, card] : cards) {
        metaCards.emplace(id, id, card, vocables);
    }

    for (const auto& card : metaCards) {
        const auto& vocableIds = card.VocableIds();
        allVocableIds.insert(vocableIds.begin(), vocableIds.end());
    }
    for (VocableId vocId : allVocableIds) {
        const auto& word = wordDB->lookupId(vocId);
        vocables->emplace(word->getId(), word->getProgress());
        // if (progressVocables.contains(vocId)) {
        //     vocables->emplace(vocId, progressVocables.at(vocId));
        // } else {
        //     vocables->emplace(vocId, VocableProgress::new_vocable);
        // }
    }
    for (const auto& [cardIndex, cardMeta] : views::enumerate(metaCards.vspan())) {
        for (const auto& vocableIndex : cardMeta.VocableIndices()) {
            (*vocables)[vocableIndex].cardIndices_insert(static_cast<std::size_t>(cardIndex));
        }
    }
    spdlog::info("number of vocables: {}", allVocableIds.size());
    spdlog::info("number of cards: {}", metaCards.size());
}

} // namespace sr
