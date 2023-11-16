#include <CardMeta.h>
#include <CardProgress.h>
#include <VocableProgress.h>
#include <annotation/CardDB.h>
#include <annotation/Ease.h>
#include <annotation/Markup.h>
#include <annotation/ZH_Tokenizer.h>
#include <dictionary/ZH_Dictionary.h>
#include <folly/sorted_vector_types.h>
#include <misc/Config.h>
#include <misc/Identifier.h>
#include <spdlog/spdlog.h>
#include <srtypes.h>
#include <utils/index_map.h>
#include <utils/min_element_val.h>

#include <algorithm>
#include <cstddef>
#include <exception>
#include <functional>
#include <iterator>
#include <map>
#include <memory>
#include <ranges>
#include <utility>
#include <vector>

#include <sys/types.h>

namespace ranges = std::ranges;
namespace views = std::views;
namespace sr {

CardMeta::CardMeta(std::shared_ptr<Card> _card,
                   std::shared_ptr<utl::index_map<VocableId, VocableMeta>> _vocables,
                   vocId_vocId_map _vocableChoices)
    : card{std::move(_card)}
    , vocables{std::move(_vocables)}
    , vocableChoices{std::move(_vocableChoices)}
{}

auto CardMeta::Id() const -> CardId
{
    return card->Id();
}

auto CardMeta::VocableIndices() const -> const index_set&
{
    if (optVocableIndices.has_value()) {
        return *optVocableIndices;
    }
    return optVocableIndices.emplace(generateVocableIndexes());
}

auto CardMeta::VocableIds() const -> const folly::sorted_vector_set<VocableId>&
{
    if (optVocableIds.has_value()) {
        return *optVocableIds;
    }
    auto vocableIds = generateVocableIDs();
    return optVocableIds.emplace(vocableIds.begin(), vocableIds.end());
}

auto CardMeta::NewVocableIds() const -> vocId_set
{
    const auto& vocableIds = VocableIds();
    vocId_set newVocableIds;
    ranges::copy_if(vocableIds,
                    std::inserter(newVocableIds, newVocableIds.begin()),
                    [this](VocableId vocId) { return not vocables->contains(vocId); });
    return newVocableIds;
}

auto CardMeta::getTimingAndVocables(bool pull) const -> const TimingAndVocables&
{
    auto& tav = pull ? timingAndVocablesPulled : timingAndVocables;
    if (tav.has_value()) {
        return *tav;
    }
    return tav.emplace(generateTimingAndVocables(pull));
}

void CardMeta::resetTimingAndVocables()
{
    timingAndVocablesPulled.reset();
    timingAndVocables.reset();
}

void CardMeta::resetAnnotation()
{
    card->resetTokenizer();
    optVocableIds.reset();
    optVocableIndices.reset();
    resetTimingAndVocables();
}

void CardMeta::addVocableChoice(VocableId oldVocId, VocableId newVocId)
{
    vocableChoices[oldVocId] = newVocId;
}

auto CardMeta::getStudyMarkup() -> std::unique_ptr<markup::Paragraph>
{
    std::vector<VocableId> vocableIds = generateVocableIDs();
    mapVocableChoices(vocableIds);
    auto studyMarkup = std::make_unique<markup::Paragraph>(card, std::move(vocableIds));
    return studyMarkup;
}

auto CardMeta::getAnnotationMarkup() -> std::unique_ptr<markup::Paragraph>
{
    auto annotationMarkup = std::make_unique<markup::Paragraph>(card);
    return annotationMarkup;
}

auto CardMeta::getRelevantEase() const -> std::map<VocableId, Ease>
{
    std::vector<VocableId> vocableIds = getActiveVocableIds();
    std::vector<Ease> eases = easesFromVocableIds(vocableIds);
    std::map<VocableId, Ease> relevantEases;

    mapVocableChoices(vocableIds);
    ranges::transform(vocableIds, eases,
                      std::inserter(relevantEases, relevantEases.begin()),
                      [](VocableId vocId, Ease ease) -> std::pair<VocableId, Ease> {
                          return {vocId, ease};
                      });
    return relevantEases;
}

auto CardMeta::generateTimingAndVocables(bool pull) const -> TimingAndVocables
{
    auto vocable_progress = [this](std::size_t vocableIndex) { return (*vocables)[vocableIndex].Progress(); };
    const auto& vocableIndices = VocableIndices();
    if (vocableIndices.empty()) {
        return {};
    }

    VocableProgress threshHoldProgress{};
    if (not pull) {
        auto progresses = vocableIndices | views::transform(vocable_progress);
        auto minIt = ranges::min_element(progresses, std::less{}, &VocableProgress::getRepeatRange);
        threshHoldProgress = *minIt;
    }
    folly::sorted_vector_set<std::size_t> nextActiveVocables;
    ranges::copy_if(vocableIndices,
                    std::inserter(nextActiveVocables, nextActiveVocables.begin()),
                    [&](const auto& vocableIndex) {
                        auto getRepeatRange = (*vocables)[vocableIndex].Progress().getRepeatRange();
                        return threshHoldProgress.getRepeatRange().implies(getRepeatRange);
                    });
    if (nextActiveVocables.empty()) {
        return {};
    }

    auto progressesNextActive = nextActiveVocables | views::transform(vocable_progress);
    auto nextActiveIt = ranges::max_element(
            progressesNextActive,
            [](const auto& range1, const auto& range2) -> bool {
                return range1.daysMin < range2.daysMin;
            },
            &VocableProgress::getRepeatRange);
    const auto& nextActiveProgress = *nextActiveIt;
    auto timing = nextActiveProgress.getRepeatRange().daysMin;
    if (timing > 0) {
        return {.timing = timing, .vocables = {}};
    }
    return {.timing = timing,
            .vocables = nextActiveVocables};
}

auto CardMeta::generateVocableIDs() const -> std::vector<VocableId>
{
    const ZH_Tokenizer& tokenizer = card->getTokenizer();
    auto vocableIds = std::vector<VocableId>{};
    ranges::transform(tokenizer.Items() | std::views::filter([](const ZH_Tokenizer::Item& item) {
                          return not item.dicItemVec.empty();
                      }),
                      std::inserter(vocableIds, vocableIds.begin()),
                      [](const ZH_Tokenizer::Item& item) -> VocableId {
                          // TODO remove static_cast
                          auto vocId = static_cast<VocableId>(item.dicItemVec.front().id);
                          return vocId;
                      });
    return vocableIds;
}

auto CardMeta::generateVocableIndexes() const -> index_set
{
    index_set result;
    const auto& vocableIds = VocableIds();
    try {
        ranges::transform(vocableIds,
                          std::inserter(result, result.begin()),
                          [this](VocableId vocableId) -> std::size_t {
                              return vocables->index_at_id(vocableId);
                          });
    } catch (const std::exception& e) {
        spdlog::error("Failed to get Vocable Indexes, error: {}", e.what());
        return {};
    }
    return result;
}

void CardMeta::mapVocableChoices(std::vector<VocableId>& vocableIds) const
{
    ranges::transform(vocableIds, vocableIds.begin(), [this](VocableId id) {
        if (vocableChoices.contains(id)) {
            return vocableChoices.at(id);
        }
        return id;
    });
}

auto CardMeta::getActiveVocableIds() const -> std::vector<VocableId>
{
    const auto& activeVocableIndices = getTimingAndVocables(true).vocables;
    std::vector<VocableId> activeVocableIds;
    ranges::transform(activeVocableIndices, std::inserter(activeVocableIds, activeVocableIds.begin()),
                      [this](size_t vocableIndex) -> VocableId {
                          return vocables->id_from_index(vocableIndex);
                      });

    return activeVocableIds;
}

auto CardMeta::easesFromVocableIds(const std::vector<VocableId>& vocableIds) const -> std::vector<Ease>
{
    std::vector<Ease> eases;
    const auto& dictionary = *card->getTokenizer().Dictionary();
    ranges::transform(
            vocableIds,
            std::back_inserter(eases),
            [&, this](VocableId vocId) -> Ease {
                const VocableProgress& vocSR = vocables->at_id(vocId).second.Progress();
                spdlog::debug("Easefactor of {} is {:.2f}, invervalDay {:.2f} - id: {}",
                              dictionary.EntryFromPosition(vocId, CharacterSetType::Simplified).key,
                              vocSR.EaseFactor(),
                              vocSR.IntervalDay(),
                              vocId);
                return {vocSR.IntervalDay(), vocSR.dueDays(), vocSR.EaseFactor()};
            });
    return eases;
}
} // namespace sr
