#pragma once
#include "Box.h"
#include "Widget.h"

#include <context/Texture.h>

#include <cstddef>
#include <initializer_list>
#include <magic_enum.hpp>
#include <string>

namespace widget {
class ToggleButtonGroup : public Widget<ToggleButtonGroup>
{
public:
    ToggleButtonGroup(WidgetInit init,
                      std::initializer_list<context::Image> images);
    ToggleButtonGroup(const WidgetInit& init,
                      std::initializer_list<std::string> labels);

    ~ToggleButtonGroup() override = default;
    ToggleButtonGroup(const ToggleButtonGroup&) = default;
    ToggleButtonGroup(ToggleButtonGroup&&) = default;
    auto operator=(const ToggleButtonGroup&) -> ToggleButtonGroup& = default;
    auto operator=(ToggleButtonGroup&&) -> ToggleButtonGroup& = default;

    auto getActive() -> std::size_t;

private:
    friend class Widget<ToggleButtonGroup>;
    auto calculateSize() const -> WidgetSize;
    std::size_t active{0};
    Box box;
};
} // namespace widget
