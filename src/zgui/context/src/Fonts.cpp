#include <Fonts.h>
#include <GlfwImguiContext.h>
#include <imgui.h>
#include <misc/Identifier.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <utility>

namespace context {
Fonts::Fonts(std::shared_ptr<GlfwImguiContext> /* _glfwImguiContext */)
{
    ImGuiIO& io = ImGui::GetIO();
    gui = io.Fonts->AddFontFromFileTTF("/home/harmen/src/zikhron/resources/IBM_Plex_Sans/IBMPlexSans-Regular.ttf",
                                       18, nullptr,
                                       io.Fonts->GetGlyphRangesDefault());
    chineseBig = io.Fonts->AddFontFromFileTTF("/usr/share/fonts/arphicfonts/gkai00mp.ttf", 50, nullptr,
                                              io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
    chineseSmall = io.Fonts->AddFontFromFileTTF("/usr/share/fonts/arphicfonts/gkai00mp.ttf", 25, nullptr,
                                                io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
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

auto Fonts::getDefaultFontColor() -> const ImVec4&
{
    return defaultFontColor;
}

auto Fonts::getShadowFontColor() -> const ImVec4&
{
    return shadowFontColor;
}

auto Fonts::getFontColor(ColorId colorId, ColorId maxColorId) -> const ImVec4&
{
    if (colorId == 0) {
        return getDefaultFontColor();
    }

    maxColorId = static_cast<ColorId>(fontColors.size());
    auto colorIndex = static_cast<ColorId>((colorId - 1) % maxColorId + 1);

    return fontColors.at(colorIndex);
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

FontColorDrop::FontColorDrop(ImVec4* fontColor) {}
void FontColorDrop::pop()
{
}
} // namespace context
