#include <Fonts.h>
#include <imgui.h>

Fonts::Fonts()
{
    ImGuiIO& io = ImGui::GetIO();
    gui = io.Fonts->AddFontFromFileTTF("/home/harmen/src/zikhron/resources/IBM_Plex_Sans/IBMPlexSans-Regular.ttf",
                                       18, nullptr,
                                       io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
    chineseBig = io.Fonts->AddFontFromFileTTF("/usr/share/fonts/arphicfonts/gkai00mp.ttf", 50, nullptr,
                                              io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
    chineseSmall = io.Fonts->AddFontFromFileTTF("/usr/share/fonts/arphicfonts/gkai00mp.ttf", 25, nullptr,
                                                io.Fonts->GetGlyphRangesChineseSimplifiedCommon());

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
