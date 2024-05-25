#include <Layer.h>
#include <context/imglog.h>
#include <detail/MetaBox.h>
#include <detail/Widget.h>

#include <algorithm>
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
    bool needArrange = false;
    auto borderedRect = getBorderedRect(rect);
    for (const auto& widget : widgets) {
        auto widgetRect = Rect{};
        auto widgetSize = widget->getWidgetSizeFromRect(borderedRect);
        if (widget->getExpandTypeWidth() == layout::width_fixed) {
            widgetRect.x = posNewWidget(widget->HorizontalAlign(), borderedRect.x, widgetSize.width, borderedRect.width);
            widgetRect.width = widgetSize.width;
        } else {
            widgetRect.x = borderedRect.x;
            widgetRect.width = borderedRect.width;
        }

        if (widget->getExpandTypeHeight() == layout::height_fixed) {
            widgetRect.y = posNewWidget(widget->VerticalAlign(), borderedRect.y, widgetSize.height, borderedRect.height);
            widgetRect.height = widgetSize.height;
        } else {
            widgetRect.y = borderedRect.y;
            widgetRect.height = borderedRect.height;
        }

        needArrange |= widget->arrange(widgetRect);
    }
    return needArrange;
}

auto Layer::getWidgetSizeFromRect(const layout::Rect& rect) -> WidgetSize
{
    winlog("mainLayer", "{}: x: {}, y: {}, w: {}, h: {}", getName(), rect.x, rect.y, rect.width, rect.height);
    auto widgetSize = WidgetSize{};
    for (const auto& widget : widgets) {
        auto ws = widget->getWidgetSizeFromRect(rect);
        winlog("mainLayer", "ml, {}: w: {}, h: {}", widget->getName(), ws.width, ws.height);
        widgetSize.width = std::max(ws.width, widgetSize.width);
        widgetSize.height = std::max(ws.height, widgetSize.height);
    }
    return widgetSize;
}

void Layer::setAlignNewWidgetsVertical(Align newWidgetsVertical)
{
    alignNewWidgetsVertical = newWidgetsVertical;
}

auto Layer::calculateSize() const -> WidgetSize
{
    return {
            .width = maxElementMeasure(widgets.begin(), widgets.end(), Measure::horizontal, SizeType::standard),
            .height = maxElementMeasure(widgets.begin(), widgets.end(), Measure::vertical, SizeType::standard),
    };
}

auto Layer::calculateMinSize() const -> WidgetSize
{
    return {
            .width = maxElementMeasure(widgets.begin(), widgets.end(), Measure::horizontal, SizeType::min),
            .height = maxElementMeasure(widgets.begin(), widgets.end(), Measure::vertical, SizeType::min),
    };
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

auto Layer::posNewWidget(Align align, float pos, float widgetSize, float rectSize) -> float
{
    auto relativePos = [=]() -> float {
        switch (align) {
        case Align::start:
            return 0;
        case Align::center:
            return (rectSize - widgetSize) / 2.F;
        case Align::end:
            return (rectSize - widgetSize);
        }
        std::unreachable();
    }();
    return relativePos + pos;
}

} // namespace widget
