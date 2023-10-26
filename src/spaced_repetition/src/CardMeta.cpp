#include <CardMeta.h>
#include <CardProgress.h>
#include <VocableProgress.h>
#include <annotation/CardDB.h>
#include <annotation/Ease.h>
#include <annotation/Markup.h>
#include <annotation/ZH_Annotator.h>
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
#include <functional>
#include <iterator>
#include <memory>
#include <ranges>
#include <utility>
#include <vector>

#include <sys/types.h>

namespace ranges = std::ranges;
namespace views = std::views;
namespace sr {

CardMeta::CardMeta(std::shared_ptr<Card> _card,
                   std::shared_ptr<utl::index_map<VocableId, VocableMeta>> _vocables)
    : card{std::move(_card)}
    , vocables{std::move(_vocables)}
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
    return generateVocableIDs();
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

auto CardMeta::getStudyMarkup() -> std::unique_ptr<markup::Paragraph>
{
    std::vector<VocableId> vocableIds;
    auto studyMarkup = std::make_unique<markup::Paragraph>(card, std::move(vocableIds));
    return studyMarkup;
}

auto CardMeta::getAnnotationMarkup() -> std::unique_ptr<markup::Paragraph>
{
    auto annotationMarkup = std::make_unique<markup::Paragraph>(card);
    return annotationMarkup;
}

auto CardMeta::getEaseList() -> std::vector<Ease>
{
    std::vector<Ease> easeList;
    return easeList;
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
    ranges::copy_if(vocableIndices, std::inserter(nextActiveVocables, nextActiveVocables.begin()),
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

auto CardMeta::generateVocableIDs() const -> const folly::sorted_vector_set<VocableId>&
{
    const ZH_Annotator& annotator = card->getAnnotator();
    auto& vocableIds = optVocableIds.emplace();
    ranges::transform(annotator.Items() | std::views::filter([](const ZH_Annotator::Item& item) {
                          return not item.dicItemVec.empty();
                      }),
                      std::inserter(vocableIds, vocableIds.begin()),
                      [](const ZH_Annotator::Item& item) -> VocableId {
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
    ranges::transform(vocableIds,
                      std::inserter(result, result.begin()),
                      [this](VocableId vocableId) -> std::size_t {
                          return vocables->index_at_id(vocableId);
                      });
    return result;
}

} // namespace sr
