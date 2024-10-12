#include "ColorSet.h"
#include "FontData.h"

#include <Fonts.h>
#include <GL/gl.h>
#include <GlfwImguiContext.h>
#include <imgui.h>
#include <misc/Identifier.h>
#include <misc/Language.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <utility>
#include <vector>

void ImGui_ImplOpenGL3_CreateFontsTexture(ImFontAtlas& fonts)
{
    GLuint tex = 0;

    // Build texture atlas
    unsigned char* pixels = nullptr;
    int width = 0;
    int height = 0;
    fonts.GetTexDataAsRGBA32(&pixels, &width, &height); // Load as RGBA 32-bit (75% of the memory is wasted, but default font is so small) because it is more likely to be compatible with user's existing shaders. If your ImTextureId represent a higher-level concept than just a GL texture id, consider calling GetTexDataAsAlpha8() instead to save on GPU memory.

    // Upload texture to graphics system
    // (Bilinear sampling is required by default. Set 'io.Fonts->Flags |= ImFontAtlasFlags_NoBakedLines' or 'style.AntiAliasedLinesUseTex = false' to allow point/nearest sampling)
    GLint last_texture = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#ifdef GL_UNPACK_ROW_LENGTH // Not on WebGL/ES
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    // Store our identifier
    fonts.SetTexID(reinterpret_cast<ImTextureID>(tex));

    // Restore state
    glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(last_texture));
}

namespace context {
auto getFontType(FontSize fontSize, Language language) -> FontType
{
    switch (fontSize) {
    case FontSize::big:
        switch (language) {
        case Language::chinese:
            return FontType::chineseBig;
        case Language::japanese:
            return FontType::japaneseBig;
        default:
            break;
        }
        break;
    case FontSize::small:
        switch (language) {
        case Language::chinese:
            return FontType::chineseSmall;
        case Language::japanese:
            return FontType::japaneseSmall;
        default:
            break;
        }
        break;
    }
    return FontType::Gui;
}

Fonts::Fonts(std::shared_ptr<GlfwImguiContext> /* _glfwImguiContext */)
{
    ImGuiIO& io = ImGui::GetIO();
    gui = io.Fonts->AddFontFromFileTTF("/home/harmen/src/zikhron/resources/IBM_Plex_Sans/IBMPlexSans-Regular.ttf",
                                       18, nullptr,
                                       io.Fonts->GetGlyphRangesDefault());
    static std::vector<ImWchar> chineseGlyphRanges = getChineseGlyphRanges();
    constexpr auto fontFileChi = "/home/harmen/zikhron/fonts/GBZenKai-Medium.ttf";
    chineseBig = io.Fonts->AddFontFromFileTTF(fontFileChi, 50, nullptr,
                                              chineseGlyphRanges.data());
    chineseSmall = io.Fonts->AddFontFromFileTTF(fontFileChi, 32, nullptr,
                                                chineseGlyphRanges.data());
    // constexpr auto fontFileJpn = "/home/harmen/zikhron/fonts/NotoSerifCJK.ttc";
    // constexpr auto fontFileJpn = "/home/harmen/zikhron/fonts/Hiragino Kaku Gothic Pro W6.otf";
    // constexpr auto fontFileJpn = "/usr/share/fonts/kochi-substitute/kochi-gothic-subst.ttf";
    fonts = std::make_unique<ImFontAtlas>();
    constexpr auto fontFileJpn = "/home/harmen/zikhron/fonts/irohamaru-Regular.ttf";
    japaneseBig = fonts->AddFontFromFileTTF(fontFileJpn, 64, nullptr,
                                            io.Fonts->GetGlyphRangesJapanese());
    japaneseSmall = fonts->AddFontFromFileTTF(fontFileJpn, 32, nullptr,
                                              io.Fonts->GetGlyphRangesJapanese());
    fonts->Build();
    ImGui_ImplOpenGL3_CreateFontsTexture(*fonts);
}

auto Fonts::dropFont(FontType fontType) const -> FontDrop
{
    switch (fontType) {
    case FontType::chineseSmall:
        return dropChineseSmall();
    case FontType::chineseBig:
        return dropChineseBig();
    case FontType::japaneseBig:
        return dropJapaneseBig();
    case FontType::japaneseSmall:
        return dropJapaneseSmall();
    case FontType::Gui:
        return dropGui();
    }
    std::unreachable();
}

auto Fonts::dropChineseBig() const -> FontDrop
{
    return {chineseBig};
}

auto Fonts::dropChineseSmall() const -> FontDrop
{
    return {chineseSmall};
}

auto Fonts::dropJapaneseBig() const -> FontDrop
{
    return {japaneseBig};
}

auto Fonts::dropJapaneseSmall() const -> FontDrop
{
    return {japaneseSmall};
}

auto Fonts::dropGui() const -> FontDrop
{
    return {Gui()};
}

auto Fonts::Gui() const -> ImFont*
{
    return gui;
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

FontColorDrop::FontColorDrop(Color fontColor)
{
    ImGui::PushStyleColor(ImGuiCol_Text, fontColor);
    incPopCount();
}

void FontColorDrop::pop()
{
    ImGui::PopStyleColor();
}
} // namespace context
