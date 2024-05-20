#include "detail/MetaBox.h"

#include <Box.h>
#include <Grid.h>
#include <Layer.h>
#include <detail/Widget.h>

#include <algorithm>
#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

namespace widget {
template<class BoxImpl>
MetaBox<BoxImpl>::MetaBox(const WidgetInit& init)
    : Widget{init}
{}

template<class BoxImpl>
void MetaBox<BoxImpl>::setPadding(float _padding)
{
    padding = _padding;
    setHorizontalPadding(padding);
    setVerticalPadding(padding);
}

template<class BoxImpl>
void MetaBox<BoxImpl>::setHorizontalPadding(float _horizontalPadding)
{
    horizontalPadding = _horizontalPadding;
}

template<class BoxImpl>
void MetaBox<BoxImpl>::setVerticalPadding(float _verticalPadding)
{
    verticalPadding = _verticalPadding;
}

template<class BoxImpl>
void MetaBox<BoxImpl>::setBorder(float _border)
{
    border = _border;
}

template<class BoxImpl>
void MetaBox<BoxImpl>::pop()
{
    auto* self = static_cast<BoxImpl*>(this);
    auto _widgetId = self->widgets.back()->getWidgetId();
    id_widgets.erase(_widgetId);
    self->widgets.pop_back();
    resetWidgetSize();
}

template<class BoxImpl>
void MetaBox<BoxImpl>::clear()
{
    auto* self = static_cast<BoxImpl*>(this);
    setArrangeIsNecessary();
    self->widgets.clear();
    id_widgets.clear();
    resetWidgetSize();
}

template<class BoxImpl>
auto MetaBox<BoxImpl>::isLast() const -> bool
{
    auto* self = static_cast<const BoxImpl*>(this);
    return currentWidgetIt == self->widgets.end();
}

template<class BoxImpl>
void MetaBox<BoxImpl>::start()
{
    auto* self = static_cast<BoxImpl*>(this);
    currentWidgetIt = self->widgets.begin();
}

template<class BoxImpl>
auto MetaBox<BoxImpl>::numberOfWidgets() const -> std::size_t
{
    auto* self = static_cast<const BoxImpl*>(this);
    return self->widgets.size();
}

template<class BoxImpl>
auto MetaBox<BoxImpl>::getBorderedRect(const layout::Rect& rect) const -> layout::Rect
{
    return {.x = rect.x + border,
            .y = rect.y + border,
            .width = rect.width - border * 2,
            .height = rect.height - border * 2};
}

template<class BoxImpl>
auto MetaBox<BoxImpl>::getBorder() const -> float
{
    return border;
}

template<class BoxImpl>
auto MetaBox<BoxImpl>::getPadding() const -> float
{
    return padding;
}

template<class BoxImpl>
auto MetaBox<BoxImpl>::getHorizontalPadding() const -> float
{
    return horizontalPadding;
}

template<class BoxImpl>
auto MetaBox<BoxImpl>::getVerticalPadding() const -> float
{
    return verticalPadding;
}

template<class BoxImpl>
auto MetaBox<BoxImpl>::getWidgetAlign(const Widget& widget, Measure measure) -> Align
{
    switch (measure) {
    case Measure::horizontal:
        return widget.HorizontalAlign();
    case Measure::vertical:
        return widget.VerticalAlign();
    }
    std::unreachable();
}

template<class BoxImpl>
void MetaBox<BoxImpl>::setWidgetAlign(Widget& widget, Measure measure, Align align)
{
    switch (measure) {
    case Measure::horizontal:
        widget.setHorizontalAlign(align);
        break;
    case Measure::vertical:
        widget.setVerticalAlign(align);
        break;
    }
}

template<class BoxImpl>
auto MetaBox<BoxImpl>::getWidgetExpandType(Widget& widget, Measure measure) -> ExpandType
{
    switch (measure) {
    case Measure::horizontal:
        return widget.getExpandTypeWidth();
    case Measure::vertical:
        return widget.getExpandTypeHeight();
    }
    std::unreachable();
}

template<class BoxImpl>
auto MetaBox<BoxImpl>::getExpandType(Measure measure) const -> ExpandType
{
    switch (measure) {
    case Measure::horizontal:
        return getExpandTypeWidth();
    case Measure::vertical:
        return getExpandTypeHeight();
    }
    std::unreachable();
}

template<class BoxImpl>
auto MetaBox<BoxImpl>::widgetSizeProjection(Measure measure, const WidgetSize& widgetSize) -> float
{
    switch (measure) {
    case Measure::horizontal:
        return widgetSize.width;
    case Measure::vertical:
        return widgetSize.height;
    }
    std::unreachable();
}

template<class BoxImpl>
auto MetaBox<BoxImpl>::rectPositionProjection(Measure measure, const layout::Rect& rect) -> float
{
    switch (measure) {
    case Measure::horizontal:
        return rect.x;
    case Measure::vertical:
        return rect.y;
    }
    std::unreachable();
}

template<class BoxImpl>
auto MetaBox<BoxImpl>::rectSizeProjection(Measure measure, const layout::Rect& rect) -> float
{
    switch (measure) {
    case Measure::horizontal:
        return rect.width;
    case Measure::vertical:
        return rect.height;
    }
    std::unreachable();
}

template<class BoxImpl>
auto MetaBox<BoxImpl>::maxElementMeasure(std::vector<std::shared_ptr<Widget>>::const_iterator first,
                                         std::vector<std::shared_ptr<Widget>>::const_iterator last,
                                         Measure measure, SizeType sizeType)
        -> float
{
    if (first == last) {
        return 0.0F;
    }
    const auto& widget = *std::max_element(first, last,
                                           [measure, sizeType](const std::shared_ptr<Widget>& widget_a,
                                                               const std::shared_ptr<Widget>& widget_b) -> bool {
                                               switch (sizeType) {
                                               case SizeType::min:
                                                   return widgetSizeProjection(measure, widget_a->getWidgetMinSize())
                                                          < widgetSizeProjection(measure, widget_b->getWidgetMinSize());
                                               case SizeType::standard:
                                                   return widgetSizeProjection(measure, widget_a->getWidgetSize())
                                                          < widgetSizeProjection(measure, widget_b->getWidgetSize());
                                               }
                                               std::unreachable();
                                           });
    switch (sizeType) {
    case SizeType::min:
        return widgetSizeProjection(measure, widget->getWidgetMinSize());
    case SizeType::standard:
        return widgetSizeProjection(measure, widget->getWidgetSize());
    }
    std::unreachable();
}

template<class BoxImpl>
auto MetaBox<BoxImpl>::maxElementMeasure(std::vector<WidgetSize>::const_iterator first,
                                         std::vector<WidgetSize>::const_iterator last,
                                         Measure measure) -> float
{
    if (first == last) {
        return 0.0F;
    }
    const auto& widgetSize = *std::max_element(first, last,
                                               [measure](const WidgetSize& widgetSize_A,
                                                         const WidgetSize& widgetSize_B) -> bool {
                                                   return widgetSizeProjection(measure, widgetSize_A)
                                                          < widgetSizeProjection(measure, widgetSize_B);
                                               });
    return widgetSizeProjection(measure, widgetSize);
}

template class MetaBox<Box>;
template class MetaBox<Grid>;
template class MetaBox<Layer>;
} // namespace widget
