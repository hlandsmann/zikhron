#include "Widget.h"

#include <memory>
#include <utility>
namespace widget {
WidgetBase::WidgetBase(layout::Align _align, std::shared_ptr<layout::Rect> _rect)
    : align{_align}
    , rect{std::move(_rect)}
{
}
auto WidgetBase::Rect() const -> const layout::Rect&
{
    return *rect;
}
} // namespace widget
