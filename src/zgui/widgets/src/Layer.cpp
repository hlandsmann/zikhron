#include <Layer.h>
#include <detail/MetaBox.h>
#include <detail/Widget.h>

#include <ranges>
#include <utility>

namespace ranges = std::ranges;
namespace views = std::ranges::views;
namespace widget {
void Layer::setup()
{
}

Layer::Layer(const WidgetInit& init)
    : MetaBox<Layer>{init}
{}

auto Layer::arrange(const layout::Rect& rect) -> bool
{
    setRect(rect);
    doLayout();

    bool needArrange = false;
    for (const auto& widget : widgets) {
        needArrange |= widget->arrange(rect);
    }
    return needArrange;
}

void Layer::setAlignNewWidgetsVertical(Align newWidgetsVertical)
{
    alignNewWidgetsVertical = newWidgetsVertical;
}

auto Layer::calculateSize() const -> WidgetSize
{
    return {
            .widthType = expandWidth,
            .heightType = expandHeight,
            .width = max_elementMeasure(widgets.begin(), widgets.end(), Measure::horizontal, SizeType::standard),
            .height = max_elementMeasure(widgets.begin(), widgets.end(), Measure::vertical, SizeType::standard),
    };
}

auto Layer::calculateMinSize() const -> WidgetSize
{
    return {
            .widthType = expandWidth,
            .heightType = expandHeight,
            .width = max_elementMeasure(widgets.begin(), widgets.end(), Measure::horizontal, SizeType::min),
            .height = max_elementMeasure(widgets.begin(), widgets.end(), Measure::vertical, SizeType::min),
    };
}

auto Layer::getChildOrientation() const -> Orientation
{
    return PassiveOrientation();
}

auto Layer::newWidgetAlign(Align align, Measure measure) const -> Align
{
    switch (measure) {
    case Measure::horizontal:
        return align;
    case Measure::vertical:
        return alignNewWidgetsVertical;
    }
    std::unreachable();
}

void Layer::doLayout()
{
    const auto& borderedRect = getBorderedRect(getRect());
    for (const auto& [widget, rect] : views::zip(widgets, rects)) {
        rect->x = posNewWidget(*widget, borderedRect, Measure::horizontal);
        rect->y = posNewWidget(*widget, borderedRect, Measure::vertical);
        rect->width = sizeNewWidget(*widget, borderedRect, Measure::horizontal);
        rect->height = sizeNewWidget(*widget, borderedRect, Measure::vertical);
    }
}

auto Layer::sizeNewWidget(const Widget& widget, const Rect& borderedRect, Measure measure) -> float
{
    const auto& widgetSize = widget.getWidgetSize();
    switch (measure) {
    case Measure::horizontal:
        switch (widgetSize.widthType) {
        case ExpandType::expand:
            return widgetSize.width;
        case ExpandType::fixed:
            return borderedRect.width;
        }
        break;

    case Measure::vertical:
        switch (widgetSize.heightType) {
        case ExpandType::expand:
            return widgetSize.height;
        case ExpandType::fixed:
            return borderedRect.height;
        }
        break;
    }
    std::unreachable();
}

auto Layer::posNewWidget(const Widget& widget, const Rect& borderedRect, Measure measure) -> float
{
    const auto& widgetSize = widget.getWidgetSize();
    switch (measure) {
    case Measure::horizontal:
        switch (widgetSize.widthType) {
        case ExpandType::expand:
            return borderedRect.x;
        case ExpandType::fixed:
            switch (widget.HorizontalAlign()) {
            case Align::start:
                return borderedRect.x;
            case Align::center:
                return borderedRect.x + (borderedRect.width - widgetSize.width) / 2;
            case Align::end:
                return borderedRect.x + (borderedRect.width - widgetSize.width);
            }
        }
        break;

    case Measure::vertical:
        switch (widgetSize.heightType) {
        case ExpandType::expand:
            return borderedRect.y;
        case ExpandType::fixed:
            switch (widget.HorizontalAlign()) {
            case Align::start:
                return borderedRect.y;
            case Align::center:
                return borderedRect.y + (borderedRect.height - widgetSize.height) / 2;
            case Align::end:
                return borderedRect.y + (borderedRect.height - widgetSize.height);
            }
        }
        break;
    }
    std::unreachable();
}
} // namespace widget
