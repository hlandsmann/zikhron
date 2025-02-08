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
    double stability = 0;
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
            .ease = 1.,
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
                       ease,
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
    auto postfix = fmt::format(" - -{} +{} - d: {:.2F}, s: {:.2F}", shiftBackward, shiftForward, ease, stability);
    if (const auto& minutes = std::chrono::duration_cast<std::chrono::minutes>(duration); minutes.count() < 60) {
        return fmt::format("{} min", minutes.count()) + postfix;
    }
    if (const auto& hours = std::chrono::duration_cast<std::chrono::hours>(duration); hours.count() < 24) {
        return fmt::format("{} hours", hours.count()) + postfix;
    }
    if (const auto& days = std::chrono::duration_cast<std::chrono::days>(duration); days.count() < 30) {
        return fmt::format("{} days", days.count()) + postfix;
    }
    if (const auto& months = std::chrono::duration_cast<Months>(duration); months.count() <= 12) {
        return fmt::format("{:.1F} months", months.count()) + postfix;
    }
    const auto& years = std::chrono::duration_cast<Years>(duration);
    return fmt::format("{:.1F} years", years.count()) + postfix;
}
} // namespace database
