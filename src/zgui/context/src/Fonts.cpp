#include <Fonts.h>
#include <GlfwImguiContext.h>
#include <imgui.h>
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

FontDrop::FontDrop(ImFont* font)
{
    ImGui::PushFont(font);
}

FontDrop::~FontDrop()
{
    ImGui::PopFont();
}
} // namespace context
