#pragma once
#include <imgui.h>

class Fonts
{
public:
    Fonts();
    [[nodiscard]] auto ChineseBig() const -> ImFont*;
    [[nodiscard]] auto ChineseSmall() const -> ImFont*;
    [[nodiscard]] auto Gui() const -> ImFont*;

private:
    ImFont* chineseBig;
    ImFont* chineseSmall;
    ImFont* gui;
};
