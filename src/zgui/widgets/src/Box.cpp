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

auto Box::arrange(const layout::Rect& rect) -> bool
{
    setRect(rect);
    if (widgets.empty()) {
        return false;
    }
    imglog::log("box_arrange, x: {}, y: {}, w: {}, h: {}", rect.x, rect.y, rect.width, rect.height);
    if (orientation == layout::Orientation::horizontal) {
        return arrange(Measure::horizontal, rect);
    }
    return arrange(Measure::vertical, rect);
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
    const auto& rect = getBorderedRect(getRect());
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

// auto Box::rectPositionProjection(const layout::Rect& rect, Measure measure) -> float
// {
//     switch (measure) {
//     case Measure::horizontal:
//         return rect.x;
//     case Measure::vertical:
//         return rect.y;
//     }
//     std::unreachable();
// }

auto Box::rectSizeProjection(Measure measure, const layout::Rect& rect) -> float
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
                               auto widgetSize = (sizeType == SizeType::min)
                                                         ? widget->getWidgetMinSize()
                                                         : widget->getWidgetSize();
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
    auto borderedRect = getBorderedRect(getRect());
    float rectSize = rectSizeProjection(measure, borderedRect);
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

auto Box::widgetNewRect(Measure measure,
                        const layout::Rect& rect,
                        float pos,
                        float size,
                        float orthogonalSize,
                        const std::shared_ptr<Widget>& widget) -> layout::Rect
{
    const Align orthogonalAlign = getWidgetAlign(oppositeMeasure(measure), widget);
    const float orthogonalPos = getWidgetCursor(oppositeMeasure(measure),
                                                Align::start, orthogonalAlign,
                                                orthogonalSize, orthogonalSize,
                                                rect, 0.F);
    return {.x = measure == Measure::horizontal ? pos : orthogonalPos,
            .y = measure == Measure::vertical ? pos : orthogonalPos,
            .width = measure == Measure::horizontal ? size : orthogonalSize,
            .height = measure == Measure::vertical ? size : orthogonalSize};
}

auto Box::rectWithAdaptedSize(Measure measure, const layout::Rect& rect, float size) -> layout::Rect
{
    return {.x = 0,
            .y = 0,
            .width = measure == Measure::horizontal
                             ? size
                             : rect.width,
            .height = measure == Measure::vertical
                              ? size
                              : rect.height};
}

auto Box::oppositeMeasure(Measure measure) -> Measure
{
    return measure == Measure::horizontal
                   ? Measure::vertical
                   : Measure::horizontal;
}

auto Box::getSizeOfWidgetSize(Measure measure, WidgetSize widgetSize) -> float
{
    switch (measure) {
    case Measure::horizontal:
        return widgetSize.width;
    case Measure::vertical:
        return widgetSize.height;
    }
    std::unreachable();
}

auto Box::getWidgetAlign(Measure measure, const std::shared_ptr<Widget>& widget) -> Align
{
    switch (measure) {
    case Measure::horizontal:
        return widget->HorizontalAlign();
    case Measure::vertical:
        return widget->VerticalAlign();
    }
    std::unreachable();
}

// void Box::setChildWidgetsInitialRect()
// {
//     auto borderedRect = getBorderedRect();
//     for (const auto& [widget, rect] : views::zip(widgets, rects)) {
//         rect->x = borderedRect.x;
//         rect->y = borderedRect.y;
//         auto widgetSize = widget->getWidgetSize();
//         rect->width = widgetSize.widthType == ExpandType::fixed ? widgetSize.width : borderedRect.width;
//         rect->height = widgetSize.heightType == ExpandType::fixed ? widgetSize.height : borderedRect.height;
//     }
// }

auto Box::getWidgetCursor(Measure measure,
                          Align oldAlign,
                          Align nextAlign,
                          float centerSize, float endSize,
                          const layout::Rect& rect,
                          float oldCursor)
        -> float
{
    if (nextAlign == oldAlign) {
        return oldCursor;
    }

    auto size = rectSizeProjection(measure, rect);
    switch (nextAlign) {
    case Align::start: // unreachable
    case Align::center: {
        float newCursor = (size - centerSize) / 2.F;
        newCursor = std::min(size - endSize - centerSize, newCursor);
        newCursor = std::max(oldCursor, newCursor);
        return newCursor;
    }
    case Align::end: {
        float newCursor = (size - endSize);
        return newCursor;
    }
    }
    std::unreachable();
}

auto Box::arrange(Measure measure, const layout::Rect& rect) -> bool
{
    const auto centerIt = ranges::find_if(widgets, [measure](const auto& widgetPtr) {
        return getWidgetAlign(measure, widgetPtr) == Align::center
               || getWidgetAlign(measure, widgetPtr) == Align::end;
    });
    const auto endIt = ranges::find_if(widgets, [measure](const auto& widgetPtr) {
        return getWidgetAlign(measure, widgetPtr) == Align::end;
    });
    const float centerSize = accumulateMeasure(centerIt, endIt, measure, SizeType::min);
    const float endSize = accumulateMeasure(endIt, widgets.end(), measure, SizeType::min);
    auto cursors = std::vector<float>{};
    cursors.reserve(widgets.size());

    float cursor = 0;
    Align align = Align::start;
    for (const auto& widget : widgets) {
        auto nextAlign = getNextAlign(align, getWidgetAlign(measure, widget));
        cursor = getWidgetCursor(measure, align, nextAlign, centerSize, endSize, rect, cursor);
        cursors.push_back(cursor);
        cursor += getSizeOfWidgetSize(measure, widget->getWidgetMinSize());
        align = nextAlign;
    }
    cursors.push_back(rectSizeProjection(measure, rect));

    auto adjacentDiff = std::vector<float>();
    adjacentDiff.reserve(widgets.size());
    std::adjacent_difference(cursors.begin(), cursors.end(), std::back_inserter(adjacentDiff));

    cursor = 0;
    cursors.clear();
    float additionalSize = 0;

    auto orthogonalSizes = std::vector<float>{};
    orthogonalSizes.reserve(widgets.size());
    for (const auto& [widget, size] : views::zip(widgets, adjacentDiff)) {
        auto widgetSize = widget->getWidgetSize(rectWithAdaptedSize(measure, rect, size + additionalSize));
        auto neededSize = getSizeOfWidgetSize(measure, widgetSize);
        auto orthogonalSize = getSizeOfWidgetSize(oppositeMeasure(measure), widgetSize);
        orthogonalSizes.push_back(orthogonalSize);
        additionalSize += size - neededSize;
        cursor += neededSize;
        cursors.push_back(cursor);
    }
    cursors.push_back(rectSizeProjection(measure, rect));
    adjacentDiff.clear();
    std::adjacent_difference(cursors.begin(), cursors.end(), std::back_inserter(adjacentDiff));

    cursors.clear();
    cursor = 0;
    align = Align::start;
    const auto centerSizeFinalIt = std::next(adjacentDiff.begin(), std::distance(widgets.begin(), centerIt));
    const auto endSizeFinalIt = std::next(adjacentDiff.begin(), std::distance(widgets.begin(), endIt));
    const float centerSizeFinal = std::accumulate(centerSizeFinalIt, endSizeFinalIt, 0.F);
    const float endSizeFinal = std::accumulate(endSizeFinalIt, adjacentDiff.end(), 0.F);
    bool needsArrange = false;
    for (const auto& [widget, size, orthogonalSize] : views::zip(widgets, adjacentDiff, orthogonalSizes)) {
        auto nextAlign = getNextAlign(align, getWidgetAlign(measure, widget));
        cursor = getWidgetCursor(measure, align, nextAlign, centerSizeFinal, endSizeFinal, rect, cursor);
        auto widgetRect = widgetNewRect(measure, rect, cursor, size, orthogonalSize, widget);
        needsArrange |= widget->arrange(widgetRect);
        cursor += getSizeOfWidgetSize(measure, widget->getWidgetSize());
    }
    return needsArrange;
}

auto Box::doLayout(Measure measure, const layout::Rect& rect) -> bool
{
    if (widgets.empty()) {
        return false;
    }
    // setChildWidgetsInitialRect();

    auto centerIt = ranges::find_if(widgets, [measure](const auto& widgetPtr) {
        return getWidgetAlign(measure, widgetPtr) == Align::center
               || getWidgetAlign(measure, widgetPtr) == Align::end;
    });
    auto endIt = ranges::find_if(widgets, [measure](const auto& widgetPtr) {
        return getWidgetAlign(measure, widgetPtr) == Align::end;
    });
    auto borderedRect = getBorderedRect(rect);
    float centerSize = accumulateMeasure(centerIt, widgets.end(), measure, SizeType::min);
    float endSize = accumulateMeasure(endIt, widgets.end(), measure, SizeType::min);
    auto widget0 = widgets.front();
    Align align0 = getNextAlign(Align::start, getWidgetAlign(measure, widget0));
    Align align1 = align0;
    float cursor0 = 0; // getWidgetNewCursor(align0,
                       //  rectPositionProjection(borderedRect, measure),
                       //  *widget0,
                       //  centerSize, endSize,
                       //  measure);

    float cursor1 = cursor0;
    for (const auto& [widget1, widgetRect0] : views::zip(std::span{std::next(widgets.begin()), widgets.end()}, rects)) {
        align1 = getNextAlign(align1, getWidgetAlign(measure, widget1)); // old Align is here also align0

        cursor1 = cursor0 + widgetSizeProjection(widget0->getWidgetMinSize(), measure) + getPadding();
        cursor1 = getWidgetNewCursor(align1, cursor1, *widget1, centerSize, endSize, measure);

        auto size = getWidgetNewSize(align0, align1, cursor0, cursor1, *widget0, measure);

        // set values
        // rectPositionProjection(*widgetRect0, measure) = cursor0;
        // rectSizeProjection(measure, *widgetRect0) = size;

        // only cursor0 needs updating for next loop
        cursor0 = cursor1;
        align0 = align1;
        widget0 = widget1;
    }
    auto& widgetRect = *rects.back();
    cursor1 = rectSizeProjection(measure, borderedRect);
    auto size = getWidgetNewSize(align0, align0, cursor0, cursor1, *widget0, measure);

    // rectPositionProjection(widgetRect, measure) = cursor0;
    // rectSizeProjection(measure, widgetRect) = size;
}

} // namespace widget
