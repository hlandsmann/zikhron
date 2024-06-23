#include <CardMeta.h>
#include <DataBase.h>
#include <VocableMeta.h>
#include <VocableProgress.h>
#include <annotation/AnnotationFwd.h>
#include <annotation/Ease.h>
#include <annotation/TokenizationChoiceDB.h>
#include <annotation/WordDB.h>
#include <dictionary/ZH_Dictionary.h>
#include <fmt/format.h>
#include <misc/Config.h>
#include <misc/Identifier.h>
#include <spdlog/spdlog.h>
#include <srtypes.h>
#include <utils/StringU8.h>
#include <utils/index_map.h>
#include <utils/min_element_val.h>

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <ranges>
#include <set>
#include <utility>

#include <sys/types.h>

namespace ranges = std::ranges;
namespace views = std::views;

namespace sr {
DataBase::DataBase(std::shared_ptr<zikhron::Config> _config)
    : config{std::move(_config)}
    // , groupDB{std::make_shared<CardAudioGroupDB>()}
    , wordDB{std::make_shared<annotation::WordDB>(config)}
    , cardPackDB{std::make_shared<CardPackDB>(config,
                                              wordDB)}
    , tokenizationChoiceDB{std::make_shared<annotation::TokenizationChoiceDB>(config)}
    , vocables{std::make_shared<utl::index_map<VocableId, VocableMeta>>()}
    , cards{std::make_shared<utl::index_map<CardId, CardMeta>>()}
{
    fillIndexMaps();
}

DataBase::~DataBase()
{
    wordDB->save();
}

void DataBase::save()
{
    wordDB->save();
}

auto DataBase::Vocables() const -> const utl::index_map<VocableId, VocableMeta>&
{
    return *vocables;
}

auto DataBase::Cards() -> utl::index_map<CardId, CardMeta>&
{
    return *cards;
}

auto DataBase::getCardPackDB() const -> std::shared_ptr<CardPackDB>
{
    return cardPackDB;
}

auto DataBase::getTokenizationChoiceDB() const -> std::shared_ptr<annotation::TokenizationChoiceDB>
{
    return tokenizationChoiceDB;
}

auto DataBase::getWordDB() const -> std::shared_ptr<WordDB>
{
    return wordDB;
}

void DataBase::reloadCard(const annotation::CardPtr& card)
{
    const auto& [cardIndex, cardMeta] = cards->at_id(card->getCardId());
    for (auto vocableIndex : cardMeta.VocableIndices()) {
        auto& vocableMeta = (*vocables)[vocableIndex];
        vocableMeta.cardIndices_erase(cardIndex);
    }
    auto tokenizationChoices = tokenizationChoiceDB->getChoicesForCard(card->getCardId());
    card->setTokenizationChoices(tokenizationChoices);
    (*cards)[cardIndex].resetMetaData();
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
    vocable.triggerByCardId(cardId);
}

void DataBase::resetCardsContainingVocable(VocableId vocId)
{
    const auto& [_, vocable] = vocables->at_id(vocId);
    const auto& cardIndices = vocable.CardIndices();
    for (size_t card_index : cardIndices) {
        auto& card = (*cards)[card_index];
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
    for (const auto& [id, card] : cardPackDB->getCards()) {
        (*cards).emplace(id, id, card.card, vocables);
    }

    for (const auto& card : *cards) {
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
    for (const auto& [cardIndex, cardMeta] : views::enumerate(cards->vspan())) {
        for (const auto& vocableIndex : cardMeta.VocableIndices()) {
            (*vocables)[vocableIndex].cardIndices_insert(static_cast<std::size_t>(cardIndex));
        }
    }
    spdlog::info("number of vocables: {}", allVocableIds.size());
    spdlog::info("number of cards: {}", cards->size());
}

} // namespace sr
