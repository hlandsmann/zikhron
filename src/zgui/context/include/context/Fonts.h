#pragma once
#include "GlfwImguiContext.h"

#include <imgui.h>

#include <memory>
namespace context {

class FontDrop;

class Fonts
{
public:
    Fonts(std::shared_ptr<GlfwImguiContext> glfwImguiContext);
    [[nodiscard]] auto dropChineseBig() const -> FontDrop;
    [[nodiscard]] auto dropChineseSmall() const -> FontDrop;
    [[nodiscard]] auto dropGui() const -> FontDrop;

private:
    [[nodiscard]] auto ChineseBig() const -> ImFont*;
    [[nodiscard]] auto ChineseSmall() const -> ImFont*;
    [[nodiscard]] auto Gui() const -> ImFont*;

    ImFont* chineseBig;
    ImFont* chineseSmall;
    ImFont* gui;
    std::shared_ptr<GlfwImguiContext> glfwImguiContext;
};

class FontDrop
{
public:
    FontDrop(ImFont* font);
    ~FontDrop();

    FontDrop(const FontDrop&) = default;
    FontDrop(FontDrop&&) = default;
    auto operator=(const FontDrop&) -> FontDrop& = default;
    auto operator=(FontDrop&&) -> FontDrop& = default;
};
} // namespace context