#include "Widget.h"

#include <spdlog/spdlog.h>

#include <memory>
#include <utility>
namespace widget {
WidgetBase::WidgetBase(layout::Orientation _orientation, layout::Align _align, std::shared_ptr<layout::Rect> _rect)
    : baseOrientation{_orientation}
    , baseAlign{_align}
    , rectPtr{std::move(_rect)}
{
}
auto WidgetBase::Rect() const -> const layout::Rect&
{
    return *rectPtr;
}

auto WidgetBase::Align() const -> layout::Align
{
    return baseAlign;
}
auto WidgetBase::Orientation() const -> layout::Orientation
{
    return baseOrientation;
}

} // namespace widget
