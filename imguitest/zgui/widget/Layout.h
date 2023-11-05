#pragma once
#include "Widget.h"

#include <imgui.h>

#include <memory>
#include <vector>

namespace widget {
namespace layout {
struct rect
{
    float x{};
    float y{};
    float width{};
    float height{};
};

enum class Orientation {
    horizontal,
    vertical
};

enum class Align {
    start,
    center,
    end
};
} // namespace layout
class Layout
{
public:
    Layout(layout::Orientation);

    template<class WidgetType, class... Args>
    auto add(layout::Align dir, Args... args) -> std::shared_ptr<WidgetType>
    {
        auto rect = std::make_shared<layout::rect>();
        auto widget = std::make_shared<WidgetType>(rect, std::forward<Args>(args)...);
        widgets.push_back(static_cast<std::shared_ptr<WidgetBase>>(widget));
        return widget;
    }

private:
    layout::Orientation orientation;
    std::vector<std::shared_ptr<WidgetBase>> widgets;
};

} // namespace widget
