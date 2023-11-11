#include "Widget.h"

#include <spdlog/spdlog.h>

#include <memory>
#include <utility>
namespace widget {
WidgetBase::WidgetBase(layout::Align _align, layout::Orientation _orientation, std::shared_ptr<layout::Rect> _rect)
    : align{_align}
    , orientation{_orientation}
    , rect{std::move(_rect)}
{
}
auto WidgetBase::Rect() const -> const layout::Rect&
{
    return *rect;
}

auto WidgetBase::Align() const -> layout::Align
{
    return align;
}
auto WidgetBase::Orientation() const -> layout::Orientation
{
    return orientation;
}

} // namespace widget
