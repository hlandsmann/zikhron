#include <Box.h>
#include <Grid.h>
#include <detail/MetaBox.h>
#include <detail/Widget.h>

#include <cstddef>

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
auto MetaBox<BoxImpl>::getBorderedRect() const -> layout::Rect
{
    const auto& rect = Rect();
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

template class MetaBox<Box>;
} // namespace widget
