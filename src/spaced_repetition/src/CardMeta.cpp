#include <CardMeta.h>
#include <CardProgress.h>
#include <VocableProgress.h>
#include <annotation/Card.h>
#include <annotation/Ease.h>
#include <annotation/ZH_Annotator.h>
#include <dictionary/ZH_Dictionary.h>
#include <folly/sorted_vector_types.h>
#include <misc/Config.h>
#include <misc/Identifier.h>
#include <spdlog/spdlog.h>
#include <utils/index_map.h>
#include <utils/min_element_val.h>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iterator>
#include <map>
#include <memory>
#include <ranges>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <sys/types.h>

namespace ranges = std::ranges;
namespace views = std::views;
namespace sr {

CardMeta::CardMeta(folly::sorted_vector_set<std::size_t> _vocableIndices,
                   std::reference_wrapper<utl::index_map<VocableId, VocableMeta>> _refVocables)
    : vocableIndices{std::move(_vocableIndices)}
    , refVocables{_refVocables}
{}

auto CardMeta::VocableIndices() const -> const folly::sorted_vector_set<std::size_t>&
{
    return vocableIndices;
}

void CardMeta::vocableIndices_insert(std::size_t vocableIndex)
{
    vocableIndices.insert(vocableIndex);
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

auto CardMeta::generateTimingAndVocables(bool pull) const -> TimingAndVocables
{
    const auto& vocables = refVocables.get();
    auto vocable_progress = [&vocables](std::size_t vocableIndex) { return vocables[vocableIndex].Progress(); };

    VocableProgress threshHoldProgress{};
    if (not pull) {
        auto progresses = vocableIndices | views::transform(vocable_progress);
        auto minIt = ranges::min_element(progresses, std::less{}, &VocableProgress::getRepeatRange);
        threshHoldProgress = *minIt;
    }
    folly::sorted_vector_set<std::size_t> nextActiveVocables;
    ranges::copy_if(vocableIndices, std::inserter(nextActiveVocables, nextActiveVocables.begin()),
                    [&](const auto& vocableIndex) {
                        auto getRepeatRange = vocables[vocableIndex].Progress().getRepeatRange();
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

} // namespace sr
