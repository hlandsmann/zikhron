#include <Box.h>
#include <Grid.h>
#include <Layer.h>
#include <detail/MetaBox.h>
#include <detail/Widget.h>

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
    self->rects.pop_back();
    self->widgets.pop_back();
    resetWidgetSize();
}

template<class BoxImpl>
void MetaBox<BoxImpl>::clear()
{
    auto* self = static_cast<BoxImpl*>(this);
    setArrangeIsNecessary();
    self->rects.clear();
    self->widgets.clear();
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
auto MetaBox<BoxImpl>::widgetSizeProjection(const WidgetSize& widgetSize, Measure measure) -> float
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
auto MetaBox<BoxImpl>::max_elementMeasure(std::vector<std::shared_ptr<Widget>>::const_iterator first,
                                          std::vector<std::shared_ptr<Widget>>::const_iterator last,
                                          Measure measure, SizeType sizeType)
        -> float
{
    if (first == last) {
        return 0.0F;
    }
    return widgetSizeProjection(
            (*std::max_element(first, last,
                               [measure, sizeType](const std::shared_ptr<Widget>& widget_a,
                                                   const std::shared_ptr<Widget>& widget_b) -> bool {
                                   switch (sizeType) {
                                   case SizeType::min:
                                       return widgetSizeProjection(widget_a->getWidgetMinSize(), measure)
                                              < widgetSizeProjection(widget_b->getWidgetMinSize(), measure);
                                   case SizeType::standard:
                                       return widgetSizeProjection(widget_a->getWidgetSize(), measure)
                                              < widgetSizeProjection(widget_b->getWidgetSize(), measure);
                                   }
                                   std::unreachable();
                               }))
                    ->getWidgetSize(),
            measure);
}

template class MetaBox<Box>;
template class MetaBox<Grid>;
template class MetaBox<Layer>;
} // namespace widget
