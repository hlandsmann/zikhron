#pragma once
#include "Drop.h"
#include "GlfwImguiContext.h"

#include <imgui.h>
#include <misc/Identifier.h>

#include <array>
#include <memory>
namespace context {

class FontDrop;
class FontColorDrop;

enum class FontType {
    chineseBig,
    chineseSmall,
    Gui,
};

constexpr auto bToColor(unsigned char r, unsigned char g, unsigned char b) -> ImVec4
{
    return {static_cast<float>(r) / 255, static_cast<float>(g) / 255, static_cast<float>(b) / 255, 1.F};
}

class Fonts
{
    static constexpr std::array<ImVec4, 11> fontColors = {bToColor(0xff, 0xe1, 0x19),
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
    static constexpr ImVec4 defaultFontColor = {1.F, 1.F, 1.F, 1.F};
    static constexpr ImVec4 shadowFontColor = {0.1F, 0.1F, 0.1F, 0.1F};

public:
    // the GlfwImguiContext needs to be initialized before this class is constructed
    Fonts(std::shared_ptr<GlfwImguiContext> /* glfwImguiContext */);
    [[nodiscard]] auto dropFont(FontType) const -> FontDrop;
    [[nodiscard]] auto dropChineseBig() const -> FontDrop;
    [[nodiscard]] auto dropChineseSmall() const -> FontDrop;
    [[nodiscard]] auto dropGui() const -> FontDrop;

    [[nodiscard]] static auto getDefaultFontColor() -> const ImVec4&;
    [[nodiscard]] static auto getShadowFontColor() -> const ImVec4&;
    [[nodiscard]] static auto getFontColor(ColorId colorId, ColorId maxColorId) -> const ImVec4&;

private:
    [[nodiscard]] auto ChineseBig() const -> ImFont*;
    [[nodiscard]] auto ChineseSmall() const -> ImFont*;
    [[nodiscard]] auto Gui() const -> ImFont*;

    ImFont* chineseBig;
    ImFont* chineseSmall;
    ImFont* gui;
    std::shared_ptr<GlfwImguiContext> glfwImguiContext;
};

class FontDrop: public Drop<FontDrop>
{
public:
    FontDrop(ImFont* font);
private:
    friend class Drop<FontDrop>;
    static void pop();
};

class FontColorDrop : public Drop<FontColorDrop>
{
public:
    FontColorDrop(ImVec4* fontColor);

private:
    friend class Drop<FontColorDrop>;
    static void pop();
};

} // namespace context
