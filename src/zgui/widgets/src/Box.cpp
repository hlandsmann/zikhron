#include "Box.h"

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
void Box::setup(Orientation _orientation)
{
    orientation = _orientation;
}

Box::Box(const WidgetInit& init)
    : MetaBox<Box>{init}
{}

auto Box::arrange(const layout::Rect& rect) -> bool
{
    // winlog("DisplayCard_box", "{}: x: {}, y: {}, w: {}, h: {}", getName(), rect.x, rect.y, rect.width, rect.height);
    // winlog("ctrlBox", "{}: x: {}, y: {}, w: {}, h: {}", getName(), rect.x, rect.y, rect.width, rect.height);
    // setRect(rect);
    auto borderedRect = getBorderedRect(rect);
    if (widgets.empty()) {
        return false;
    }
    // imglog::log("box_arrange, x: {}, y: {}, w: {}, h: {}", rect.x, rect.y, rect.width, rect.height);
    if (orientation == Orientation::horizontal) {
        return arrange(Measure::horizontal, borderedRect);
    }
    return arrange(Measure::vertical, borderedRect);
}

void Box::setOrthogonalAlign(Align align)
{
    orthogonalAlign = align;
}

auto Box::calculateSize(SizeType sizeType) const -> WidgetSize
{
    WidgetSize result{};
    switch (orientation) {
    case Orientation::horizontal:
        result = {
                .width = accumulateMeasure(widgets.begin(), widgets.end(), Measure::horizontal, sizeType),
                .height = max_elementMeasure(widgets.begin(), widgets.end(), Measure::vertical, sizeType),
        };
        break;
    case Orientation::vertical:
        result = {
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

auto Box::newWidgetAlign(Align align, Measure measure) const -> Align
{
    if ((orientation == Orientation::horizontal && measure == Measure::horizontal)
        || (orientation == Orientation::vertical && measure == Measure::vertical)) {
        return align;
    }
    return orthogonalAlign;
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
                               return val + widgetSizeProjection(measure, widgetSize) + ((val == 0.F) ? 0.F : getPadding());
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
                             ? std::min(rect.width, size)
                             : rect.width,
            .height = measure == Measure::vertical
                              ? std::min(rect.height, size)
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

auto Box::getWidgetCursor(Measure measure,
                          Align oldAlign,
                          Align nextAlign,
                          float centerSize, float endSize,
                          const layout::Rect& rect,
                          float oldCursor)
        -> float
{
    float minCursor = rectPositionProjection(measure, rect);
    oldCursor = std::max(minCursor, oldCursor);
    if (nextAlign == oldAlign) {
        return oldCursor;
    }

    // normalize
    oldCursor -= minCursor;
    float newCursor{};

    auto size = rectSizeProjection(measure, rect);
    switch (nextAlign) {
    case Align::start: // unreachable
    case Align::center: {
        newCursor = (size - centerSize) / 2.F;
        newCursor = std::min(size - endSize - centerSize, newCursor);
        newCursor = std::max(oldCursor, newCursor);
        break;
    }
    case Align::end: {
        newCursor = (size - endSize);
        break;
    }
    }
    // un-normalize
    newCursor += minCursor;
    return newCursor;
}

auto Box::arrange(Measure measure, const layout::Rect& rect) -> bool
{
    // imglog::log("{}:, x: {}, y: {}", getName(), rect.x, rect.y);
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

    float cursor = rectPositionProjection(measure, rect);
    Align align = Align::start;
    for (const auto& widget : widgets) {
        auto nextAlign = getNextAlign(align, getWidgetAlign(measure, widget));
        cursor = getWidgetCursor(measure, align, nextAlign, centerSize, endSize, rect, cursor);
        cursors.push_back(cursor);
        cursor += getSizeOfWidgetSize(measure, widget->getWidgetMinSize());
        // winlog("linebox", "{}, {}", getName(), cursor);
        // winlog("cardBox", "{}, {}", getName(), cursor);
        align = nextAlign;
    }
    cursors.push_back(rectSizeProjection(measure, rect));

    auto adjacentDiff = std::vector<float>();
    adjacentDiff.reserve(widgets.size() + 1);
    std::adjacent_difference(cursors.begin(), cursors.end(), std::back_inserter(adjacentDiff));
    adjacentDiff.erase(adjacentDiff.begin());
    // for (const auto& c : cursors) {
    //     imglog::log("{}: cursor: {}", getName(), c);
    // }
    // for (const auto& d : adjacentDiff) {
    //     imglog::log("diff: {}, ", d);
    // }
    // imglog::log("centerSize: {}, endSize: {}", centerSize, endSize);

    cursor = rectPositionProjection(measure, rect);
    cursors.clear();
    float additionalSize = 0;

    auto sizes = std::vector<float>();
    sizes.reserve(widgets.size());
    auto orthogonalSizes = std::vector<float>{};
    orthogonalSizes.reserve(widgets.size());
    for (const auto& [widget, size] : views::zip(widgets, adjacentDiff)) {
        auto widgetSize = widget->getWidgetSizeFromRect(rectWithAdaptedSize(measure, rect, size + additionalSize));
        auto neededSize = getSizeOfWidgetSize(measure, widgetSize);
        auto orthogonalSize = getSizeOfWidgetSize(oppositeMeasure(measure), widgetSize);
        winlog("cardBox", "{}, o:{}, w: {}", getName(), orthogonalSize, rect.width);
        orthogonalSizes.push_back(orthogonalSize);
        additionalSize += size - neededSize;
        cursors.push_back(cursor);
        cursor += neededSize;
        sizes.push_back(neededSize);
        // imglog::log("cursor: {}, size: {}, ns: {}, os: {}, #: {}", cursor, size, neededSize, orthogonalSize, widgets.size());
    }
    cursors.push_back(rectSizeProjection(measure, rect));
    adjacentDiff.clear();
    std::adjacent_difference(cursors.begin(), cursors.end(), std::back_inserter(adjacentDiff));
    adjacentDiff.erase(adjacentDiff.begin());

    cursors.clear();
    cursor = rectPositionProjection(measure, rect);
    align = Align::start;
    const auto centerSizeFinalIt = std::next(sizes.begin(), std::distance(widgets.begin(), centerIt));
    const auto endSizeFinalIt = std::next(sizes.begin(), std::distance(widgets.begin(), endIt));
    const float centerSizeFinal = std::accumulate(centerSizeFinalIt, endSizeFinalIt, 0.F);
    const float endSizeFinal = std::accumulate(endSizeFinalIt, sizes.end(), 0.F);
    // imglog::log("centerSizeFinal: {}, endSizeFinal: {}", centerSizeFinal, endSizeFinal);
    bool needsArrange = false;
    for (const auto& [widget, size, orthogonalSize] : views::zip(widgets, sizes, orthogonalSizes)) {
        auto nextAlign = getNextAlign(align, getWidgetAlign(measure, widget));
        cursor = getWidgetCursor(measure, align, nextAlign, centerSizeFinal, endSizeFinal, rect, cursor);
        // imglog::log("size: {}, cursor: {}, align: {}", size, cursor, static_cast<int>(nextAlign));
        auto widgetRect = widgetNewRect(measure, rect, cursor, size, orthogonalSize, widget);
        needsArrange |= widget->arrange(widgetRect);
        cursor += getSizeOfWidgetSize(measure, widget->getWidgetSize());
        align = nextAlign;
    }
    return needsArrange;
}

} // namespace widget
