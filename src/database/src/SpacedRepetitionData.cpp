#include "SpacedRepetitionData.h"

#include "SrsWeights.h"

#include <utils/Time.h>
#include <utils/format.h>

#include <cmath>
#include <numbers>
#include <string>
#include <string_view>

namespace database {
auto SpacedRepetitionData::fromVocableProgress(const VocableProgress& progress, const SrsWeights& srsWeights) -> SpacedRepetitionData
{
    double decay = -0.5;
    double factor = std::pow(0.9, (1. / decay)) - 1;
    double desiredRetention = 0.9;
    // double interval = ((srd.stability / factor) * std::pow(desiredRetention, 1 / decay)) - 1;
    double stability = ((progress.intervalDay + 1) * factor) / std::pow(desiredRetention, 1 / decay);
    using namespace std::numbers;
    if (!progress.isEnabled()) {
        return {};
    }
    auto repRange = progress.getRepeatRange();

    return {
            .reviewed = progress.lastSeen,
            .due = utl::advanceTimeByDays(progress.lastSeen, progress.intervalDay),
            .shiftBackward = repRange.daysNormal - repRange.daysMin,
            .shiftForward = repRange.daysMax - repRange.daysNormal,
            .difficulty = 10.,//srsWeights[4] - std::pow(e, srsWeights[5]) + 1,
            // .difficulty = srsWeights[4] - std::pow(e, srsWeights[5]) + 1,
            // self.parameters[4] - (math.e ** (self.parameters[5] * (rating - 1))) + 1
            // .stability = srsWeights[Weight::initialStability_ratingGood],
            .stability = stability,
            .state = StudyState::review,
            .triggerCardIndices = progress.triggerCardIndices,
            .enabled = progress.enabled,

    };
}

auto SpacedRepetitionData::serialize() const -> std::string
{
    return fmt::format("{},{},{:.4F},{:.4F},{},{},{},{},{},",
                       utl::serialize_time_t(reviewed),
                       utl::serialize_time_t(due),
                       difficulty,
                       stability,
                       state,
                       shiftBackward,
                       shiftForward,
                       enabled,
                       fmt::join(triggerCardIndices, ","));
}

} // namespace database
