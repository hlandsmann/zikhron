#include "ColorSet.h"

#include <misc/Identifier.h>

#include <cstddef>
#include <utility>
#include <vector>

namespace context {

ColorSet::ColorSet()
    : vocableCount_11{getDefaultColorSet(ColorSetId::vocableCount_11)}
    , adjacentAlternate{getDefaultColorSet(ColorSetId::adjacentAlternate)}
{}

auto ColorSet::getColor(ColorId colorId, ColorSetId colorSetId) const -> Color
{
    if (colorId == ColorId::defaultFontColor) {
        return defaultFontColor;
    }
    if (colorId == ColorId::shadowFontColor) {
        return shadowFontColor;
    }
    const auto& colors = [this, colorSetId]() -> const std::vector<Color>& {
        switch (colorSetId) {
        case ColorSetId::vocableCount_11:
            return vocableCount_11;
        case ColorSetId::adjacentAlternate:
            return adjacentAlternate;
        }
        std::unreachable();
    }();

    auto maxColorId = static_cast<ColorId>(colors.size());
    auto colorIndex = static_cast<ColorId>(colorId % maxColorId);

    return colors.at(colorIndex);
}

auto ColorSet::getColorSetId(std::size_t /* vocableCount */) -> ColorSetId
{
    return ColorSetId::vocableCount_11;
}

auto ColorSet::getDefaultColorSet(ColorSetId colorSetId) -> std::vector<Color>
{
    constexpr auto bToColor = [](unsigned char r, unsigned char g, unsigned char b)
            -> Color {
        return {static_cast<float>(r) / 255, static_cast<float>(g) / 255, static_cast<float>(b) / 255, 1.F};
    };

    switch (colorSetId) {
    case ColorSetId::vocableCount_11:
        return {bToColor(0xff, 0xe1, 0x19),
                bToColor(0x3c, 0xd4, 0x4b),
                bToColor(0x42, 0xd4, 0xf4),
                bToColor(0xf5, 0x82, 0x31),
                bToColor(0xf0, 0x32, 0xe6),
                bToColor(0xfa, 0xbe, 0xd4),
                bToColor(0x91, 0x3e, 0xc4),
                bToColor(0xff, 0x29, 0x4B),
                bToColor(0x43, 0x63, 0xff),
                bToColor(0xbf, 0xef, 0x45),
                bToColor(0x46, 0x99, 0x90)};
    case ColorSetId::adjacentAlternate:
        return {bToColor(0xdd, 0x40, 0x40),
                bToColor(0xef, 0xA0, 0xA0),
                bToColor(0x40, 0xcc, 0x40),
                bToColor(0xA0, 0xef, 0xA0)};
    }
    std::unreachable();
}

} // namespace context
