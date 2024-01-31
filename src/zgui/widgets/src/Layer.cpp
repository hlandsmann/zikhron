#include <Layer.h>
#include <context/imglog.h>
#include <detail/MetaBox.h>
#include <detail/Widget.h>

#include <algorithm>
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
    bool needArrange = false;
    auto borderedRect = getBorderedRect(rect);
    for (const auto& widget : widgets) {
        auto widgetRect = Rect{};
        auto widgetSize = widget->getWidgetSizeFromRect(borderedRect);
        widgetRect.x = posNewWidget(widget->HorizontalAlign(), widgetSize.width, widgetRect.width);
        widgetRect.y = posNewWidget(widget->VerticalAlign(), widgetSize.height, widgetRect.height);
        widgetRect.width = widgetSize.width;
        widgetRect.height = widgetSize.height;

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
            .width = max_elementMeasure(widgets.begin(), widgets.end(), Measure::horizontal, SizeType::standard),
            .height = max_elementMeasure(widgets.begin(), widgets.end(), Measure::vertical, SizeType::standard),
    };
}

auto Layer::calculateMinSize() const -> WidgetSize
{
    return {
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

auto Layer::posNewWidget(Align align, float widgetSize, float rectSize) -> float
{
    switch (align) {
    case Align::start:
        return 0;
    case Align::center:
        return (rectSize - widgetSize) / 2.F;
    case Align::end:
        return (rectSize - widgetSize);
    }
    std::unreachable();
}

// auto Layer::sizeNewWidget(const Widget& widget, const Rect& borderedRect, Measure measure) -> float
// {
//     const auto& widgetSize = widget.getWidgetSize();
//     switch (measure) {
//     case Measure::horizontal:
//         switch (widgetSize.widthType) {
//         case ExpandType::expand:
//             return widgetSize.width;
//         case ExpandType::fixed:
//             return borderedRect.width;
//         }
//         break;
//
//     case Measure::vertical:
//         switch (widgetSize.heightType) {
//         case ExpandType::expand:
//             return widgetSize.height;
//         case ExpandType::fixed:
//             return borderedRect.height;
//         }
//         break;
//     }
//     std::unreachable();
// }
//
// auto Layer::posNewWidget(const Widget& widget, const Rect& borderedRect, Measure measure) -> float
// {
//     const auto& widgetSize = widget.getWidgetSize();
//     switch (measure) {
//     case Measure::horizontal:
//         switch (widgetSize.widthType) {
//         case ExpandType::expand:
//             return borderedRect.x;
//         case ExpandType::fixed:
//             switch (widget.HorizontalAlign()) {
//             case Align::start:
//                 return borderedRect.x;
//             case Align::center:
//                 return borderedRect.x + (borderedRect.width - widgetSize.width) / 2;
//             case Align::end:
//                 return borderedRect.x + (borderedRect.width - widgetSize.width);
//             }
//         }
//         break;
//
//     case Measure::vertical:
//         switch (widgetSize.heightType) {
//         case ExpandType::expand:
//             return borderedRect.y;
//         case ExpandType::fixed:
//             switch (widget.HorizontalAlign()) {
//             case Align::start:
//                 return borderedRect.y;
//             case Align::center:
//                 return borderedRect.y + (borderedRect.height - widgetSize.height) / 2;
//             case Align::end:
//                 return borderedRect.y + (borderedRect.height - widgetSize.height);
//             }
//         }
//         break;
//     }
//     std::unreachable();
// }
} // namespace widget
