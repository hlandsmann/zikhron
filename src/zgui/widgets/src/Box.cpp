#include <Box.h>
#include <context/Theme.h>
#include <context/imglog.h>
#include <detail/MetaBox.h>
#include <detail/Widget.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <memory>
#include <numeric>
#include <ranges>
#include <span>
#include <utility>
#include <vector>

namespace ranges = std::ranges;
namespace views = std::ranges::views;
namespace widget {
Box::Box(const WidgetInit& init)
    : MetaBox<Box>{init}
    , orientation{init.orientation}
{}

auto Box::arrange() -> bool
{
    if (orientation == layout::Orientation::horizontal) {
        doLayout(Measure::horizontal);
    } else {
        doLayout(Measure::vertical);
    }
    bool needArrange = false;
    for (const auto& widget : widgets) {
        needArrange |= widget->arrange();
    }
    return needArrange;
}

void Box::setOrientationHorizontal()
{
    orientation = layout::Orientation::horizontal;
}

void Box::setOrientationVertical()
{
    orientation = layout::Orientation::vertical;
}

void Box::setFlipChildrensOrientation(bool flip)
{
    flipChildrensOrientation = flip;
}

void Box::setOrthogonalAlign(Align align)
{
    orthogonalAlign = align;
}

auto Box::getExpandedSize() const -> WidgetSize
{
    const auto& rect = getBorderedRect();
    auto widgetSize = getWidgetSize();
    widgetSize.width = rect.width;
    widgetSize.height = rect.height;
    return widgetSize;
}

auto Box::calculateSize(SizeType sizeType) const -> WidgetSize
{
    WidgetSize result{};
    using layout::Orientation;
    switch (orientation) {
    case Orientation::horizontal:
        result = {
                .widthType = expandWidth,
                .heightType = expandHeight,
                .width = accumulateMeasure(widgets.begin(), widgets.end(), Measure::horizontal, sizeType),
                .height = max_elementMeasure(widgets.begin(), widgets.end(), Measure::vertical, sizeType),
        };
        break;
    case Orientation::vertical:
        result = {
                .widthType = expandWidth,
                .heightType = expandHeight,
                .width = max_elementMeasure(widgets.begin(), widgets.end(), Measure::horizontal, sizeType),
                .height = accumulateMeasure(widgets.begin(), widgets.end(), Measure::vertical, sizeType)};

        break;
    }
    result.width += getBorder() * 2;
    result.height += getBorder() * 2;
    return result;
}

auto Box::calculateSize() const -> WidgetSize
{
    return calculateSize(SizeType::standard);
}

auto Box::calculateMinSize() const -> WidgetSize
{
    return calculateSize(SizeType::min);
}

auto Box::getChildOrientation() const -> Orientation
{
    if (!flipChildrensOrientation) {
        return PassiveOrientation();
    }
    return PassiveOrientation() == layout::Orientation::vertical
                   ? Orientation::horizontal
                   : Orientation::vertical;
}

auto Box::newWidgetAlign(Align align, Measure measure) const -> Align
{
    if ((PassiveOrientation() == Orientation::horizontal && measure == Measure::horizontal)
        || (PassiveOrientation() == Orientation::vertical && measure == Measure::vertical)) {
        return align;
    }
    return orthogonalAlign;
}

auto Box::widgetExpandTypeProjection(const WidgetSize& widgetSize, Measure measure) -> ExpandType
{
    switch (measure) {
    case Measure::horizontal:
        return widgetSize.widthType;
    case Measure::vertical:
        return widgetSize.heightType;
    }
    std::unreachable();
}

auto Box::rectPositionProjection(layout::Rect& rect, Measure measure) -> float&
{
    switch (measure) {
    case Measure::horizontal:
        return rect.x;
    case Measure::vertical:
        return rect.y;
    }
    std::unreachable();
}

auto Box::rectSizeProjection(layout::Rect& rect, Measure measure) -> float&
{
    switch (measure) {
    case Measure::horizontal:
        return rect.width;
    case Measure::vertical:
        return rect.height;
    }
    std::unreachable();
}

auto Box::accumulateMeasure(std::vector<std::shared_ptr<Widget>>::const_iterator first,
                            std::vector<std::shared_ptr<Widget>>::const_iterator last,
                            Measure measure, SizeType sizeType) const -> float
{
    return std::accumulate(first, last, float{},
                           [this, measure, sizeType](float val, const std::shared_ptr<Widget>& widget) -> float {
                               auto widgetSize = (sizeType == SizeType::min) ? widget->getWidgetMinSize() : widget->getWidgetSize();
                               return val + widgetSizeProjection(widgetSize, measure) + ((val == 0.F) ? 0.F : getPadding());
                           });
}

auto Box::getNextAlign(Align oldAlign, Align nextAlign)
{
    switch (oldAlign) {
    case Align::start:
        return nextAlign;
    case Align::center:
        if (nextAlign == Align::start) {
            return Align::center;
        }
        return nextAlign;
    case Align::end:
        return Align::end;
    }
    std::unreachable();
}

auto Box::getWidgetNewCursor(Align align, float cursor, const Widget& widget,
                             float centerSize, float endSize, Measure measure) const -> float
{
    auto borderedRect = getBorderedRect();
    float rectSize = rectSizeProjection(borderedRect, measure);
    float widgetSize = widgetSizeProjection(widget.getWidgetSize(), measure);
    switch (align) {
    case Align::start:
        return cursor;
    case Align::center: {
        float centerCursor = rectSize / 2 - widgetSize / 2;
        float minCursor = std::min(rectSize - centerSize, centerCursor);
        return std::max(minCursor, cursor);
    }
    case Align::end:
        float maxCursor = std::max(rectSize - endSize, cursor);
        if (widgetExpandTypeProjection(widget.getWidgetSize(), measure) == ExpandType::fixed
            || widgetSize < endSize) {
            return maxCursor;
        }
        return cursor;
    }
    std::unreachable();
}

auto Box::getWidgetNewSize(Align align, Align alignNextWidget,
                           float cursor, float cursorNextWidget,
                           const Widget& widget,
                           Measure measure) const -> float
{
    float projectedSize = widgetSizeProjection(widget.getWidgetSize(), measure);
    ExpandType projectedExpandType = widgetExpandTypeProjection(widget.getWidgetSize(), measure);
    switch (align) {
    case Align::start:
    case Align::center:
        if (align == alignNextWidget || projectedExpandType == ExpandType::fixed) {
            return projectedSize;
        }
        return cursorNextWidget - cursor - getPadding();
    case Align::end:
        if (projectedExpandType == ExpandType::fixed) {
            return projectedSize;
        }
        return cursorNextWidget - cursor;
    }
    std::unreachable();
}

void Box::setChildWidgetsInitialRect()
{
    auto borderedRect = getBorderedRect();
    for (const auto& [widget, rect] : views::zip(widgets, rects)) {
        rect->x = borderedRect.x;
        rect->y = borderedRect.y;
        auto widgetSize = widget->getWidgetSize();
        rect->width = widgetSize.widthType == ExpandType::fixed ? widgetSize.width : borderedRect.width;
        rect->height = widgetSize.heightType == ExpandType::fixed ? widgetSize.height : borderedRect.height;
    }
}

void Box::doLayout(Measure measure)
{
    if (widgets.empty()) {
        return;
    }
    setChildWidgetsInitialRect();

    auto centerIt = ranges::find_if(widgets, [](const auto& widgetPtr) {
        return widgetPtr->HorizontalAlign() == Align::center
               || widgetPtr->HorizontalAlign() == Align::end;
    });
    auto endIt = ranges::find_if(widgets, [](const auto& widgetPtr) {
        return widgetPtr->HorizontalAlign() == Align::end;
    });
    auto borderedRect = getBorderedRect();
    float centerSize = accumulateMeasure(centerIt, widgets.end(), measure, SizeType::min);
    float endSize = accumulateMeasure(endIt, widgets.end(), measure, SizeType::min);
    auto widget0 = widgets.front();
    Align align0 = getNextAlign(Align::start, widget0->HorizontalAlign());
    Align align1 = align0;
    float cursor0 = getWidgetNewCursor(align0,
                                       rectPositionProjection(borderedRect, measure),
                                       *widget0,
                                       centerSize, endSize,
                                       measure);

    float cursor1 = cursor0;
    for (const auto& [widget1, widgetRect0] : views::zip(std::span{std::next(widgets.begin()), widgets.end()}, rects)) {
        align1 = getNextAlign(align1, widget1->HorizontalAlign()); // old Align is here also align0

        cursor1 = cursor0 + widgetSizeProjection(widget0->getWidgetMinSize(), measure) + getPadding();
        cursor1 = getWidgetNewCursor(align1, cursor1, *widget1, centerSize, endSize, measure);

        auto size = getWidgetNewSize(align0, align1, cursor0, cursor1, *widget0, measure);

        // set values
        rectPositionProjection(*widgetRect0, measure) = cursor0;
        rectSizeProjection(*widgetRect0, measure) = size;

        // only cursor0 needs updating for next loop
        cursor0 = cursor1;
        align0 = align1;
        widget0 = widget1;
    }
    auto& widgetRect = *rects.back();
    cursor1 = rectSizeProjection(borderedRect, measure);
    auto size = getWidgetNewSize(align0, align0, cursor0, cursor1, *widget0, measure);

    rectPositionProjection(widgetRect, measure) = cursor0;
    rectSizeProjection(widgetRect, measure) = size;
}

} // namespace widget
