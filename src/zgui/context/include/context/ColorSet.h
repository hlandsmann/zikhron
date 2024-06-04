#pragma once
#include <imgui.h>
#include <misc/Identifier.h>

#include <cstddef>
#include <vector>

namespace context {

using Color = ImVec4;
enum class ColorSetId {
    vocableCount_11,
    adjacentAlternate,
};

class ColorSet
{
public:
    ColorSet();
    [[nodiscard]] auto getColor(ColorId colorId, ColorSetId colorSetId) const -> Color;
    [[nodiscard]] static auto getColorSetId(std::size_t /* vocableCount */) -> ColorSetId;

private:
    [[nodiscard]] static auto getDefaultColorSet(ColorSetId colorSetId) -> std::vector<Color>;

    std::vector<Color> vocableCount_11;
    std::vector<Color> adjacentAlternate;
    Color defaultFontColor{1.F, 1.F, 1.F, 1.F};
    Color shadowFontColor = {0.1F, 0.1F, 0.1F, 1.0F};
};

} // namespace context
