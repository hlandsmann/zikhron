#pragma once
#include "Widget.h"

#include <context/Theme.h>
#include <imgui.h>

#include <memory>
#include <string>

namespace widget {
enum class ib {
    cards,
};

class Button : public Widget<Button>
{
public:
    Button(std::shared_ptr<context::Theme> theme,
           layout::Orientation orientation,
           layout::Align align,
           std::shared_ptr<layout::Rect> rect,
           std::string label);
    ~Button() override = default;

    Button(const Button&) = default;
    Button(Button&&) = default;
    auto operator=(const Button&) -> Button& = default;
    auto operator=(Button&&) -> Button& = default;

    auto clicked() const -> bool;

private:
    friend class Widget<Button>;
    auto calculateSize() const -> WidgetSize;

    std::string label;
};

}; // namespace widget