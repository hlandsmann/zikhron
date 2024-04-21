#include <CardMeta.h>
#include <DataBase.h>
#include <VocableMeta.h>
#include <VocableProgress.h>
#include <annotation/CardDB.h>
#include <annotation/Ease.h>
#include <annotation/WordDB.h>
#include <annotation/ZH_Tokenizer.h>
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
#include <exception>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <ranges>
#include <set>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <sys/types.h>

namespace ranges = std::ranges;
namespace views = std::views;

namespace sr {
DataBase::DataBase(std::shared_ptr<zikhron::Config> _config)
    : config{std::move(_config)}
    , wordDB{std::make_shared<annotation::WordDB>(config)}
    , cardDB{std::make_shared<CardDB>(config,
                                      wordDB)}
    , vocables{std::make_shared<utl::index_map<VocableId, VocableMeta>>()}
    , cards{std::make_shared<utl::index_map<CardId, CardMeta>>()}
{
    fillIndexMaps();
}


auto DataBase::Vocables() const -> const utl::index_map<VocableId, VocableMeta>&
{
    return *vocables;
}

auto DataBase::Cards() -> utl::index_map<CardId, CardMeta>&
{
    return *cards;
}

auto DataBase::getWordDB() const -> std::shared_ptr<WordDB>
{
    return wordDB;
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
    for (const auto& [id, cardPtr] : cardDB->get()) {
        (*cards).emplace(id, cardPtr, vocables);
    }

    for (const auto& card : *cards) {
        const auto& vocableIds = card.VocableIds();
        allVocableIds.insert(vocableIds.begin(), vocableIds.end());
    }
    for (VocableId vocId : allVocableIds) {
        if (progressVocables.contains(vocId)) {
            vocables->emplace(vocId, progressVocables.at(vocId));
        } else {
            vocables->emplace(vocId, VocableProgress::new_vocable);
        }
    }
    for (const auto& [cardIndex, cardMeta] : views::enumerate(cards->vspan())) {
        for (const auto& vocableIndex : cardMeta.VocableIndices()) {
            (*vocables)[vocableIndex].cardIndices_insert(static_cast<std::size_t>(cardIndex));
        }
    }
    spdlog::info("number of vocables: {}", allVocableIds.size());
    spdlog::info("number of cards: {}", cards->size());
}

void DataBase::addNewVocableIds(const vocId_set& newVocableIds)
{
    for (VocableId newVocId : newVocableIds) {
        vocables->emplace(newVocId, VocableProgress::new_vocable);
    }
}

} // namespace sr
