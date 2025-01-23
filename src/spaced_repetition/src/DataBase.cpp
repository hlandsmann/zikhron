#include <CardMeta.h>
#include <DataBase.h>
#include <VocableMeta.h>
#include <annotation/Ease.h>
#include <database/CbdFwd.h>
#include <database/TokenizationChoiceDB.h>
#include <database/TokenizationChoiceDbChi.h>
#include <database/VocableProgress.h>
#include <database/WordDB.h>
#include <dictionary/DictionaryChi.h>
#include <misc/Config.h>
#include <misc/Identifier.h>
#include <spdlog/spdlog.h>
#include <srtypes.h>
#include <utils/StringU8.h>
#include <utils/format.h>
#include <utils/index_map.h>
#include <utils/min_element_val.h>

#include <algorithm>
#include <iterator>
#include <map>
#include <memory>
#include <ranges>
#include <set>
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
    if (auto tokenizationChoiceDbChi = std::dynamic_pointer_cast<TokenizationChoiceDbChi>(tokenizationChoiceDB)) {
        tokenizationChoiceDbChi->save();
    }
    cardDB->save();
}

auto DataBase::Vocables() const -> const utl::index_map<VocableId, VocableMeta>&
{
    return *vocables;
}

auto DataBase::MetaCards() -> const std::map<CardId, CardMeta>&
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

auto DataBase::getCardMeta(const database::CardPtr& card) -> CardMeta
{
    auto cardId = card->getCardId();
    if (cardExists(cardId)) {
        return metaCards.at(cardId);
    }
    auto cardMeta = CardMeta{cardId, card, vocables};
    for (VocableId vocId : cardMeta.VocableIds()) {
        if (vocables->contains(vocId)) {
            continue;
        }
        const auto& word = wordDB->lookupId(vocId);
        vocables->emplace(word->getId(), word->getProgress());
    }
    return cardMeta;
}

void DataBase::reloadCard(const database::CardPtr& card)
{
    auto cardId = card->getCardId();
    const auto& cardMeta = metaCards.at(card->getCardId());
    for (auto vocableIndex : cardMeta.VocableIndices()) {
        auto& vocableMeta = (*vocables)[vocableIndex];
        vocableMeta.eraseCardId(cardId);
    }
    setTokenizationChoiceForCard(card);
    metaCards[cardId].resetMetaData();
    addVocablesOfCardMeta(cardMeta);
    for (auto vocableIndex : cardMeta.VocableIndices()) {
        auto& vocableMeta = (*vocables)[vocableIndex];
        vocableMeta.insertCardId(cardId);
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
    vocable.triggerByCardId(cardId);
}

void DataBase::resetCardsContainingVocable(VocableId vocId)
{
    const auto& [_, vocable] = vocables->at_id(vocId);
    for (CardId cardId : vocable.CardIds()) {
        auto& card = metaCards.at(cardId);
        card.resetTimingAndVocables();
    }
}

auto DataBase::cardExists(CardId cardId) const -> bool
{
    return metaCards.contains(cardId);
}

void DataBase::addCard(const database::CardPtr& card)
{
    auto cardId = card->getCardId();
    metaCards[cardId] = CardMeta{cardId, card, vocables};
    const auto& cardMeta = metaCards.at(cardId);
    addVocablesOfCardMeta(cardMeta);
    for (const auto& vocableIndex : cardMeta.VocableIndices()) {
        (*vocables)[vocableIndex].insertCardId(cardId);
    }
    // spdlog::error("added!, {}", fmt::ptr(&cardMeta));
    card->setActive(true);
    cardDB->addCard(card);
    spdlog::info("Added card with cardId: {}", cardId);
}

void DataBase::removeCard(CardId cardId)
{
    const auto& cardMeta = metaCards.at(cardId);
    auto card = cardMeta.getCard();
    for (const auto& vocableIndex : cardMeta.VocableIndices()) {
        (*vocables)[vocableIndex].eraseCardId(cardId);
    }
    metaCards.erase(cardId);

    card->setActive(false);
    cardDB->eraseCard(cardId);
    spdlog::info("Removed card with cardId: {}", cardId);
}

void DataBase::cleanupCards()
{
    cardId_set deadCards;
    for (const auto& [cardId, cardMeta] : metaCards) {
        if (!cardMeta.getCard()->isActive()) {
            deadCards.insert(cardId);
        }
    }
    for (CardId cardId : deadCards) {
        removeCard(cardId);
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
    setTokenizationChoiceForCardAllCards();
    for (const auto& [id, card] : cards) {
        metaCards[id] = CardMeta{id, card, vocables};
    }

    for (const auto& [_, card] : metaCards) {
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
    for (const auto& [cardId, cardMeta] : metaCards) {
        for (const auto& vocableIndex : cardMeta.VocableIndices()) {
            (*vocables)[vocableIndex].insertCardId(cardId);
        }
    }
    
    std::size_t enabledVocables=0;
    for (auto& vocableMeta : *vocables) {
        if (vocableMeta.CardIds().size() > 1) {
            vocableMeta.setEnabled(true);
            enabledVocables++;
        }
    }
    spdlog::info("number of vocables: {}, enabled: {}", allVocableIds.size(), enabledVocables);
    spdlog::info("number of cards: {}", metaCards.size());
}

void DataBase::addVocablesOfCardMeta(const CardMeta& cardMeta)
{
    for (VocableId vocId : cardMeta.VocableIds()) {
        if (vocables->contains(vocId)) {
            continue;
        }
        const auto& word = wordDB->lookupId(vocId);
        vocables->emplace(word->getId(), word->getProgress());
    }
}

void DataBase::setTokenizationChoiceForCard(const database::CardPtr& card) const
{
    if (auto tokenizationChoiceDbChi = std::dynamic_pointer_cast<TokenizationChoiceDbChi>(tokenizationChoiceDB)) {
        auto tokenizationChoices = tokenizationChoiceDbChi->getChoicesForCard(card->getCardId());
        card->setTokenizationChoices(tokenizationChoices);
    }
}

void DataBase::setTokenizationChoiceForCardAllCards() const
{
    if (auto tokenizationChoiceDbChi = std::dynamic_pointer_cast<TokenizationChoiceDbChi>(tokenizationChoiceDB)) {
        const std::map<CardId, database::CardPtr>& cards = cardDB->getCards();
        for (const auto& [cardId, choices] : tokenizationChoiceDbChi->getChoicesForCards()) {
            if (!cards.contains(cardId)) {
                continue;
            }
            const auto& card = cards.at(cardId);

            card->setTokenizationChoices(choices);
        }
    }
}

} // namespace sr
