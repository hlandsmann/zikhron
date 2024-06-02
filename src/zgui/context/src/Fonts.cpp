#include <Fonts.h>
#include <GlfwImguiContext.h>
#include <imgui.h>
#include <misc/Identifier.h>
#include <spdlog/spdlog.h>

#include <cstddef>
#include <memory>
#include <utility>

namespace context {
auto ChineseFull() -> const ImWchar*
{
    // clang-format off
    static const ImWchar ranges[] = 
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x0100, 0x01DC, // PINYIN
        0x2000, 0x206F, // General Punctuation
        0x3000, 0x30FF, // CJK Symbols and Punctuations, Hiragana, Katakana
        0x31F0, 0x31FF, // Katakana Phonetic Extensions
        0xFF00, 0xFFEF, // Half-width characters
        0xFFFD, 0xFFFD, // Invalid
        0x4e00, 0x9FAF, // CJK Ideograms
        0,
    }; // clang-format on
    return &ranges[0];
}

Fonts::Fonts(std::shared_ptr<GlfwImguiContext> /* _glfwImguiContext */)
{
    ImGuiIO& io = ImGui::GetIO();
    gui = io.Fonts->AddFontFromFileTTF("/home/harmen/src/zikhron/resources/IBM_Plex_Sans/IBMPlexSans-Regular.ttf",
                                       18, nullptr,
                                       io.Fonts->GetGlyphRangesDefault());
    chineseBig = io.Fonts->AddFontFromFileTTF("/usr/share/fonts/arphicfonts/gkai00mp.ttf", 50, nullptr,
                                              io.Fonts->GetGlyphRangesChineseFull());
    chineseSmall = io.Fonts->AddFontFromFileTTF("/usr/share/fonts/arphicfonts/gkai00mp.ttf", 25, nullptr,
                                                ChineseFull());
}

auto Fonts::dropFont(FontType fontType) const -> FontDrop
{
    switch (fontType) {
    case FontType::chineseSmall:
        return dropChineseSmall();
    case FontType::chineseBig:
        return dropChineseBig();
    case FontType::Gui:
        return dropGui();
    }
    std::unreachable();
}

auto Fonts::dropChineseBig() const -> FontDrop
{
    return {ChineseBig()};
}

auto Fonts::dropChineseSmall() const -> FontDrop
{
    return {ChineseSmall()};
}

auto Fonts::dropGui() const -> FontDrop
{
    return {Gui()};
}

auto Fonts::dropDefaultFontColor() const -> FontColorDrop
{
    return getDefaultFontColor();
}

auto Fonts::dropShadowFontColor() const -> FontColorDrop
{
    return getShadowFontColor();
}

auto Fonts::ChineseBig() const -> ImFont*
{
    return chineseBig;
}

auto Fonts::ChineseSmall() const -> ImFont*
{
    return chineseSmall;
}

auto Fonts::Gui() const -> ImFont*
{
    return gui;
}

auto Fonts::getDefaultFontColor() const -> const ImVec4&
{
    return defaultFontColor;
}

auto Fonts::getShadowFontColor() const -> const ImVec4&
{
    return shadowFontColor;
}

auto Fonts::getFontColor(ColorId colorId, ColorId maxColorId) const -> const ImVec4&
{
    if (colorId == 0) {
        return getDefaultFontColor();
    }

    maxColorId = static_cast<ColorId>(fontColors.size());
    auto colorIndex = static_cast<ColorId>((colorId - 1) % maxColorId);

    return fontColors.at(colorIndex);
}

auto Fonts::getFontColorAlternative(std::size_t indexAlt, bool alt) const -> const ImVec4&
{
    auto index = indexAlt % (fontColorsAlternatives.size() / 2);
    index = index * 2 + (alt ? 1 : 0);
    return fontColorsAlternatives.at(index);
}

FontDrop::FontDrop(ImFont* font)
{
    ImGui::PushFont(font);
    incPopCount();
}

void FontDrop::pop()
{
    ImGui::PopFont();
}

FontColorDrop::FontColorDrop(ImVec4 fontColor)
{
    ImGui::PushStyleColor(ImGuiCol_Text, fontColor);
    incPopCount();
}

void FontColorDrop::pop()
{
    ImGui::PopStyleColor();
}
} // namespace context
