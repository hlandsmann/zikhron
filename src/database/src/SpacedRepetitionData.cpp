#include "SpacedRepetitionData.h"

#include "SrsWeights.h"

#include <utils/Time.h>
#include <utils/format.h>
#include <utils/string_split.h>

#include <chrono>
#include <cmath>
#include <cstddef>
#include <magic_enum.hpp>
#include <numbers>
#include <ratio>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace database {
auto SpacedRepetitionData::fromVocableProgress(const VocableProgress& progress, const SrsWeights& srsWeights) -> SpacedRepetitionData
{
    // double stability = 0;
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
            .stability = std::log(static_cast<double>(progress.intervalDay)) / std::log(2),
            .state = StudyState::review,
            .enabled = progress.enabled,
            .triggerCardIndices = progress.triggerCardIndices,

    };
}

auto SpacedRepetitionData::serialize() const -> std::string
{
    // time_point reviewed;
    // time_point due;
    // int shiftBackward{};
    // int shiftForward{};
    // double ease{};
    // double stability{};
    // StudyState state{StudyState::newWord};
    // bool enabled{false};
    // std::vector<std::size_t> triggerCardIndices;
    return fmt::format("{},{},{},{},{:.4F},{:.4F},{},{},{},",
                       utl::serializeTimePoint(reviewed),
                       utl::serializeTimePoint(due),
                       shiftBackward,
                       shiftForward,
                       ease,
                       stability,
                       state,
                       enabled,
                       fmt::join(triggerCardIndices, ","));
}

auto SpacedRepetitionData::deserialize(std::string_view sv) -> SpacedRepetitionData
{
    return {
            .reviewed = utl::deserializeTimePoint(std::string{utl::split_front(sv, ',')}),
            .due = utl::deserializeTimePoint(std::string{utl::split_front(sv, ',')}),
            .shiftBackward = std::stoi(std::string{utl::split_front(sv, ',')}),
            .shiftForward = std::stoi(std::string{utl::split_front(sv, ',')}),
            .ease = std::stof(std::string{utl::split_front(sv, ',')}),
            .stability = std::stof(std::string{utl::split_front(sv, ',')}),
            .state = [&sv]() -> StudyState {
                const auto& studyStateString = std::string{utl::split_front(sv, ',')};
                auto optStudyState = magic_enum::enum_cast<StudyState>(studyStateString);
                if (!optStudyState.has_value()) {
                    throw std::runtime_error(fmt::format("desialize spaced repitiion data, unknown study state: {}",
                                                         studyStateString));
                }
                return optStudyState.value();
            }(),
            .enabled = std::string{utl::split_front(sv, ',')} == "true",
            .triggerCardIndices = [&sv]() -> std::vector<std::size_t> {
                std::vector<std::size_t> triggerCardIndices;
                while (true) {
                    auto cardIndex = std::string{utl::split_front(sv, ',')};
                    if (cardIndex.empty()) {
                        break;
                    }
                    triggerCardIndices.push_back(std::stoul(cardIndex));
                }
                return triggerCardIndices;
            }(),
    };
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
