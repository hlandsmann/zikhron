#pragma once
#include "SrsWeights.h"
#include "VocableProgress.h"

#include <utils/format.h>

#include <cstddef>
#include <ctime>
#include <magic_enum.hpp>
#include <string>
#include <string_view>
#include <vector>

namespace database {
enum class StudyState {
    newWord,
    learning,
    review,
    relearning,
};

struct SpacedRepetitionData
{
    [[nodiscard]] auto serialize() const -> std::string;
    static auto deserialize(std::string_view sv) -> SpacedRepetitionData;
    static auto fromVocableProgress(const VocableProgress& progress, const SrsWeights& srsWeights) -> SpacedRepetitionData;

    std::time_t reviewed{};
    std::time_t due{};
    int shiftBackward{};
    int shiftForward{};
    double difficulty{};
    double stability{};
    StudyState state{};

    std::vector<std::size_t> triggerCardIndices;
    bool enabled{false};
};
} // namespace database

template<>
struct fmt::formatter<database::StudyState>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(database::StudyState state, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "{}", magic_enum::enum_name(state));
    }
};
