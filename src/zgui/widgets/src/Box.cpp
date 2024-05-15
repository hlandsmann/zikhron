#include "Box.h"

#include <context/Theme.h>
#include <context/imglog.h>
#include <detail/MetaBox.h>
#include <detail/Widget.h>
#include <spdlog/spdlog.h>
#include <utils/spdlog.h>

#include <algorithm>
#include <cstddef>
#include <deque>
#include <functional>
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
    // winlog("linebox_ttq_1", "{}: x: {}, y: {}, w: {}, h: {}", getName(), rect.x, rect.y, rect.width, rect.height);
    // winlog("linebox_ttq_1", "{}: size: {}", getName(), widgets.size());
    // imglog::log("{}: size: {}", getName(), widgets.size());
    // imglog::log("box, {}: x: {}, y: {}, w: {}, h: {}, widgetsSize(): {}", getName(), rect.x, rect.y, rect.width, rect.height, widgets.size());
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

auto Box::rectWithAdaptedPosSize(Measure measure,
                                 const layout::Rect& rect,
                                 float cursor,
                                 float size, float orthogonalSize) -> layout::Rect
{
    if (orthogonalSize == 0) {
        orthogonalSize = measure == Measure::horizontal
                                 ? rect.height
                                 : rect.width;
    }
    return {.x = measure == Measure::horizontal
                         ? std::max(rect.x, cursor)
                         : rect.x,
            .y = measure == Measure::vertical
                         ? std::max(rect.y, cursor)
                         : rect.y,
            .width = measure == Measure::horizontal
                             ? size
                             : orthogonalSize,
            .height = measure == Measure::vertical
                              ? size
                              : orthogonalSize};
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
    auto sizes = std::vector<float>(widgets.size(), 0.F);
    bool needArrange = true;
    auto sizesFmt = fmt::format("{}", fmt::join(sizes, ", "));
    // imglog::log("cursors: {} --------------------------", logstr);

    traverseWidgets(rect, measure, sizes,
                    [](const std::shared_ptr<Widget>& widget, float /* cursor */, float /* size */, float /* availableSize */)
                            -> WidgetSize {
                        return widget->getWidgetMinSize();
                    });
    sizesFmt = fmt::format("{}", fmt::join(sizes, ", "));
    // imglog::log("box: {}, w: {}, h: {}, sizes: {}", getName(), rect.width, rect.height, sizesFmt);
    winlog("tbgrp", "box: {}, w: {}, h: {}, sizes: {}", getName(), rect.width, rect.height, sizesFmt);

    auto orthogonalSizes = std::deque<float>{};

    traverseWidgets(rect, measure, sizes,
                    [&, this](const std::shared_ptr<Widget>& widget, float cursor, float /* size */, float availableSize)
                            -> WidgetSize {
                        // imglog::log("box1: {}, cursor: {}, size: {}", getName(), cursor, size);
                        winlog("tbgrp", "box1: {}, cursor: {}, size: {}", getName(), cursor, availableSize);
                        const auto& widgetRect = rectWithAdaptedPosSize(measure, rect, cursor, availableSize);
                        const auto& widgetSize = widget->getWidgetSizeFromRect(widgetRect);
                        orthogonalSizes.push_back(widgetSizeProjection(oppositeMeasure(measure), widgetSize));
                        return widgetSize;
                    });
    sizesFmt = fmt::format("{}", fmt::join(sizes, ", "));
    winlog("tbgrp", "box: {}, sizes: {}", getName(), sizesFmt);
    traverseWidgets(rect, measure, sizes,
                    [&](const std::shared_ptr<Widget>& widget, float cursor, float size, float /* availableSize */)
                            -> WidgetSize {
                        float orthogonalSize = orthogonalSizes.front();
                        orthogonalSizes.pop_front();
                        const auto& widgetRect = rectWithAdaptedPosSize(measure, rect, cursor, size, orthogonalSize);
                        needArrange = widget->arrange(widgetRect);
                        auto widgetSize = widget->getWidgetSize();
                        return widgetSize;
                    });

    return needArrange;
}

auto Box::getAvailableSize(float fullSize, float startSize, float centerSize, float endSize, Align align) -> float
{
    if (centerSize == 0.F) {
        return fullSize - (startSize + endSize);
    }

    switch (align) {
    case Align::start:
        return (fullSize - centerSize) / 2 - startSize;
    case Align::center:
        return fullSize - std::max(startSize, endSize) * 2;
    case Align::end:
        return (fullSize - centerSize) / 2 - endSize;
    }
    std::unreachable();
}

void Box::traverseWidgets(const layout::Rect& rect,
                          Measure measure,
                          std::vector<float>& sizes,
                          std::function<WidgetSize(
                                  const std::shared_ptr<Widget>& widget,
                                  float cursor,
                                  float size,
                                  float availableSize)>
                                  fun) const
{
    const auto widgetCenterIt = ranges::find_if(widgets, [measure](const auto& widgetPtr) {
        return getWidgetAlign(measure, widgetPtr) == Align::center;
    });
    const auto widgetEndIt = ranges::find_if(widgets, [measure](const auto& widgetPtr) {
        return getWidgetAlign(measure, widgetPtr) == Align::end;
    });
    auto centerIndex = std::distance(widgets.begin(), widgetCenterIt);
    auto endIndex = std::distance(widgets.begin(), widgetEndIt);
    centerIndex = std::min(centerIndex, endIndex);

    // spdlog::info("{}: {}, {}, size: {}", getName(), centerIndex, endIndex, sizes.size());
    const auto sizeCenterIt = std::next(sizes.begin(), centerIndex);
    const auto sizeEndIt = std::next(sizes.begin(), endIndex);

    float fullSize = rectSizeProjection(measure, rect); // std::accumulate(sizes.begin(), sizes.end(), 0.F);
    fullSize -= getPadding() * static_cast<float>(widgets.size() - 1);
    // spdlog::info("{}: fullSize:{}", getName(), fullSize);
    float startSize = std::accumulate(sizes.begin(), sizeCenterIt, 0.F);
    // startSize += getPadding() * static_cast<float>(std::min(centerIndex, static_cast<long>(widgets.size() - 1)));
    // spdlog::info("{}: startSize:{}", getName(), startSize);
    float centerSize = std::accumulate(sizeCenterIt, sizeEndIt, 0.F);
    // spdlog::info("{}: centerSize:{}", getName(), centerSize);
    float endSize = std::accumulate(sizeEndIt, sizes.end(), 0.F);
    // spdlog::info("{}: endSize:{}", getName(), endSize);
    // float cursor = rectPositionProjection(measure, rect);
    float cursor = 0.F;
    auto align = Align::start;
    for (const auto& [index, widget] : views::enumerate(widgets)) {
        auto nextAlign = getNextAlign(align, getWidgetAlign(measure, widget));

        cursor = getWidgetCursor(measure, align, nextAlign, centerSize, endSize, rect, cursor);

        auto& currentSize = sizes.at(static_cast<std::size_t>(index));
        auto& alignedSize = (nextAlign == Align::start)    ? startSize
                            : (nextAlign == Align::center) ? centerSize
                                                           : endSize;
        alignedSize -= currentSize;
        float availableSize = getAvailableSize(fullSize, startSize, centerSize, endSize, align);
        const auto& widgetSize = fun(widget, cursor, currentSize, availableSize);
        currentSize = widgetSizeProjection(measure, widgetSize);
        alignedSize += currentSize;

        cursor += currentSize + getPadding();
        align = nextAlign;
    }
}

} // namespace widget
