#include "SpacedRepetitionData.h"

#include "SrsWeights.h"

#include <utils/Time.h>
#include <utils/format.h>

#include <chrono>
#include <cmath>
#include <numbers>
#include <ratio>
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
            .reviewed = std::chrono::system_clock::from_time_t(progress.lastSeen),
            .due = std::chrono::system_clock::from_time_t(utl::advanceTimeByDays(progress.lastSeen, progress.intervalDay)),
            .shiftBackward = repRange.daysNormal - repRange.daysMin,
            .shiftForward = repRange.daysMax - repRange.daysNormal,
            .difficulty = 8., // srsWeights[4] - std::pow(e, srsWeights[5]) + 1,
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
                       utl::serializeTimePoint(reviewed),
                       utl::serializeTimePoint(due),
                       difficulty,
                       stability,
                       state,
                       shiftBackward,
                       shiftForward,
                       enabled,
                       fmt::join(triggerCardIndices, ","));
}

auto SpacedRepetitionData::getDueInTimeLabel() const -> std::string
{
    auto duration = due - reviewed;
    using Months = std::chrono::duration<double, std::ratio<2629746>>;
    using Years = std::chrono::duration<double, std::ratio<31556952>>;

    if (const auto& minutes = std::chrono::duration_cast<std::chrono::minutes>(duration); minutes.count() < 60) {
        return fmt::format("{} min", minutes.count());
    }
    if (const auto& hours = std::chrono::duration_cast<std::chrono::hours>(duration); hours.count() < 24) {
        return fmt::format("{} hours", hours.count());
    }
    if (const auto& days = std::chrono::duration_cast<std::chrono::days>(duration); days.count() < 30) {
        return fmt::format("{} days", days.count());
    }
    if (const auto& months = std::chrono::duration_cast<Months>(duration); months.count() <= 12) {
        return fmt::format("{:.1F} months", months.count());
    }
    const auto& years = std::chrono::duration_cast<Years>(duration);
    return fmt::format("{:.1F} years", years.count());
}
} // namespace database
