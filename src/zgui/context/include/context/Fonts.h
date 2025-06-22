#pragma once
#include "ColorSet.h"
#include "Drop.h"
#include "GlfwImguiContext.h"

#include <imgui.h>
#include <misc/Identifier.h>
#include <misc/Language.h>

#include <memory>

namespace context {

class FontDrop;
class FontColorDrop;

enum class FontSize {
    big,
    small
};

enum class FontType {
    chineseBig,
    chineseSmall,
    japaneseBig,
    japaneseSmall,
    Gui,
};

[[nodiscard]] auto getFontType(FontSize fontSize, Language language) -> FontType;

constexpr auto bToColor(unsigned char r, unsigned char g, unsigned char b) -> ImVec4
{
    return {static_cast<float>(r) / 255, static_cast<float>(g) / 255, static_cast<float>(b) / 255, 1.F};
}

class Fonts
{
public:
    // the GlfwImguiContext needs to be initialized before this class is constructed
    Fonts(std::shared_ptr<GlfwImguiContext> /* glfwImguiContext */);
    [[nodiscard]] auto dropFont(FontType) const -> FontDrop;
    [[nodiscard]] auto dropChineseBig() const -> FontDrop;
    [[nodiscard]] auto dropChineseSmall() const -> FontDrop;
    [[nodiscard]] auto dropJapaneseBig() const -> FontDrop;
    [[nodiscard]] auto dropJapaneseSmall() const -> FontDrop;
    [[nodiscard]] auto dropGui() const -> FontDrop;

private:
    [[nodiscard]] auto Gui() const -> ImFont*;

    ImFont* chineseBig;
    ImFont* chineseSmall;
    ImFont* japaneseBig;
    ImFont* japaneseSmall;
    ImFont* gui;
    std::shared_ptr<GlfwImguiContext> glfwImguiContext;
    std::unique_ptr<ImFontAtlas> fonts;
};

class FontDrop : public Drop<FontDrop>
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
    FontColorDrop(Color fontColor);

private:
    friend class Drop<FontColorDrop>;
    static void pop();
};

} // namespace context
