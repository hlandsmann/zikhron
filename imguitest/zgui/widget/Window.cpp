#include "Window.h"

#include "Layout.h"
#include "Widget.h"

#include <imgui.h>

#include <memory>
#include <string>
#include <utility>

namespace widget {
Window::Window(std::shared_ptr<layout::rect> _rect, float _width, float _height, std::string _name)
    : rect{std::move(_rect)}
    , width{_width}
    , height{_height}
    , name{std::move(_name)}

{}

auto Window::calculateSize() const -> WidgetSize
{
    return {.widthType = (width == 0.F) ? size_type::variable : size_type::fixed,
            .heightType = (height == 0.F) ? size_type::variable : size_type::fixed,
            .width = width,
            .height = height};
}

} // namespace widget
