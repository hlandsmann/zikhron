#include "Box.h"

#include <context/Theme.h>
#include <context/imglog.h>
#include <detail/MetaBox.h>
#include <detail/Widget.h>
#include <utils/format.h>

#include <algorithm>
#include <cstddef>
#include <deque>
#include <functional>
#include <iterator>
#include <memory>
#include <numeric>
#include <ranges>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace ranges = std::ranges;
namespace views = std::ranges::views;

namespace widget {
void Box::setup(Orientation _orientation)
{
    orientation = _orientation;
}

void Box::setup(const BoxCfg& boxCfg, Orientation _orientation)
{
    setCfg(boxCfg);
    setup(_orientation);
}

Box::Box(const WidgetInit& init)
    : MetaBox<Box>{init}
{}

auto Box::arrange(const layout::Rect& rect) -> bool
{
    const std::string& winName = "overlayBox";
    const std::string& parentName = "overlayBox";
    if (widgets.empty()) {
        return false;
    }
    bool needArrange = false;

    auto widgetSizes = std::vector<WidgetSize>(widgets.size(), WidgetSize{});
    calculateWidgetSizes(rect, widgetSizes);

    const auto& borderedRect = getBorderedRect(rect);
    float lastCursorX{};
    float lastCursorY{};
    traverseWidgets(borderedRect, widgetSizes,
                    [&, this](const std::shared_ptr<Widget>& widget,
                              float cursorX, float cursorY,
                              float width, float height,
                              float availableWidth, float availableHeight,
                              const WidgetSize& widgetSize) -> WidgetSize {
                        auto measure = orientation == Orientation::horizontal
                                               ? Measure::horizontal
                                               : Measure::vertical;
                        // imglog::log("box1: {}, cursor: {}, size: {}", getName(), cursor, size);
                        lastCursorX = cursorX;
                        lastCursorY = cursorY;
                        auto expandWidth = widget->getExpandTypeWidth();
                        auto expandHeight = widget->getExpandTypeHeight();
                        auto widgetRect = layout::Rect{
                                .x = cursorX,
                                .y = cursorY,
                                .width = expandWidth == ExpandType::fixed   ? widgetSize.width
                                         : expandWidth == ExpandType::adapt ? width
                                                                            : availableWidth,
                                .height = expandHeight == ExpandType::fixed   ? widgetSize.height
                                          : expandHeight == ExpandType::adapt ? height
                                                                              : availableHeight};
                        if (measure == Measure::horizontal && expandHeight != ExpandType::fixed) {
                            widgetRect.height = availableHeight;
                        }
                        if (measure == Measure::vertical && expandWidth != ExpandType::fixed) {
                            widgetRect.width = availableWidth;
                        }

                        needArrange |= widget->arrange(widgetRect);
                        return widget->getWidgetSize();
                    });
    lastCursorX -= rect.x;
    lastCursorY -= rect.y;
    auto measure = orientation == Orientation::horizontal ? Measure::horizontal : Measure::vertical;
    float width = getExpandTypeWidth() != ExpandType::fixed ? rect.width
                  : measure == Measure::horizontal          ? lastCursorX + widgetSizes.back().width
                                                            : maxElementMeasure(widgetSizes.begin(), widgetSizes.end(),
                                                                                oppositeMeasure(measure));
    width += getBorder() * 2;
    float height = getExpandTypeHeight() != ExpandType::fixed ? rect.height
                   : measure == Measure::vertical             ? lastCursorY + widgetSizes.back().height
                                                              : maxElementMeasure(widgetSizes.begin(), widgetSizes.end(),
                                                                                  oppositeMeasure(measure));
    height += getBorder() * 2;
    if (width < 0 || height < 0) {
        imglog::log("{} -------- w: {}, h: {}, ew: {}, eh: {}, o: {}, rx: {}, ry: {}, rw: {}, rh: {}, number: {}",
                    getName(), width, height,
                    fmt::format("{}", getExpandTypeWidth()),
                    fmt::format("{}", getExpandTypeHeight()),
                    fmt::format("{}", orientation),
                    rect.x, rect.y, rect.width, rect.height,
                    widgets.size());
        scratchDbg();
    }
    boxWidgetSize = {.width = std::min(width, rect.width),
                     .height = std::min(height, rect.height)};
    return needArrange;
}

void Box::setOrthogonalAlign(Align align)
{
    orthogonalAlign = align;
}

auto Box::calculateSize() const -> WidgetSize
{
    if (widgets.empty()) {
        return {};
    }
    if (boxWidgetSize.width == 0 || boxWidgetSize.height == 0) {
        return getWidgetMinSize();
    }
    return boxWidgetSize;
}

auto Box::calculateMinSize() const -> WidgetSize
{
    if (widgets.empty()) {
        return {};
    }
    WidgetSize result{};
    auto widgetSizes = std::vector<WidgetSize>{};
    ranges::transform(widgets, std::back_inserter(widgetSizes),
                      [](const std::shared_ptr<Widget>& widget) {
                          return widget->getWidgetMinSize();
                      });
    switch (orientation) {
    case Orientation::horizontal:
        result = {
                .width = accumulateMeasure(widgetSizes.begin(), widgetSizes.end(), Measure::horizontal),
                .height = maxElementMeasure(widgetSizes.begin(), widgetSizes.end(), Measure::vertical),
        };
        break;
    case Orientation::vertical:
        result = {
                .width = maxElementMeasure(widgetSizes.begin(), widgetSizes.end(), Measure::horizontal),
                .height = accumulateMeasure(widgetSizes.begin(), widgetSizes.end(), Measure::vertical),
        };

        break;
    }
    result.width += getBorder() * 2;
    result.height += getBorder() * 2;

    return result;
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

auto Box::accumulateMeasure(std::vector<WidgetSize>::const_iterator first,
                            std::vector<WidgetSize>::const_iterator last,
                            Measure measure) const -> float
{
    return std::accumulate(first, last, float{},
                           [this, measure](float val, const WidgetSize& widgetSize) -> float {
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

auto Box::oppositeMeasure(Measure measure) -> Measure
{
    return measure == Measure::horizontal
                   ? Measure::vertical
                   : Measure::horizontal;
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

auto Box::getWidgetSizeFromRect(const layout::Rect& rect) -> WidgetSize
{
    if (widgets.empty()) {
        return {};
    }
    auto widgetSizes = std::vector<WidgetSize>(widgets.size(), WidgetSize{});
    calculateWidgetSizes(rect, widgetSizes);
    auto measure = orientation == Orientation::horizontal ? Measure::horizontal : Measure::vertical;

    float width = getExpandTypeWidth() == ExpandType::expand ? rect.width
                  : measure == Measure::horizontal           ? accumulateMeasure(widgetSizes.begin(), widgetSizes.end(), measure)
                                                             : maxElementMeasure(widgetSizes.begin(), widgetSizes.end(),
                                                                                 oppositeMeasure(measure));
    if (getExpandTypeWidth() != ExpandType::expand) {
        width += getBorder() * 2;
    }

    float height = getExpandTypeHeight() == ExpandType::expand ? rect.height
                   : measure == Measure::vertical              ? accumulateMeasure(widgetSizes.begin(), widgetSizes.end(), measure)
                                                               : maxElementMeasure(widgetSizes.begin(), widgetSizes.end(),
                                                                                   oppositeMeasure(measure));
    if (getExpandTypeHeight() != ExpandType::expand) {
        height += getBorder() * 2;
    }

    width = std::min(width, rect.width);
    height = std::min(height, rect.height);
    if (anyParentHasName("scratchBox")) {
        parentlog("overlayBox", "getname: {} rwsfr --- w: {}, h: {}, rw: {}, rh: {}", getName(), width, height,
                  rect.width, rect.height);
    }
    return {.width = width,
            .height = height};
}

void Box::calculateWidgetSizes(const layout::Rect& rect,
                               std::vector<WidgetSize>& widgetSizes) const
{
    const auto& borderedRect = getBorderedRect(rect);
    traverseWidgets(borderedRect, widgetSizes,
                    [](const std::shared_ptr<Widget>& widget,
                       float /* cursorX */, float /* cursorY */,
                       float /* width */, float /* height */,
                       float /* availableWidth */, float /* availableHeight */,
                       const WidgetSize& /* widgetSize */)
                            -> WidgetSize {
                        return widget->getWidgetMinSize();
                    });

    traverseWidgets(borderedRect, widgetSizes,
                    [&](const std::shared_ptr<Widget>& widget,
                        float /* cursorX */, float /* cursorY */,
                        float /* width */, float /* height */,
                        float availableWidth, float availableHeight,
                        const WidgetSize& /* widgetSize */)
                            -> WidgetSize {
                        // imglog::log("box1: {}, cursor: {}, size: {}", getName(), cursor, size);
                        const auto& widgetRect = layout::Rect{
                                .x = 0.F,
                                .y = 0.F,
                                .width = availableWidth,
                                .height = availableHeight};
                        const auto& widgetSize = widget->getWidgetSizeFromRect(widgetRect);
                        return widgetSize;
                    });
}

void Box::traverseWidgets(const layout::Rect& rect,
                          std::vector<WidgetSize>& widgetSizes,
                          std::function<WidgetSize(
                                  const std::shared_ptr<Widget>& widget,
                                  float cursorX, float cursorY,
                                  float width, float height,
                                  float availableWidth, float availableHeight,
                                  const WidgetSize& widgetSize)>
                                  fun) const
{
    auto measure = orientation == Orientation::horizontal ? Measure::horizontal : Measure::vertical;
    const auto widgetCenterIt = ranges::find_if(widgets, [measure](const auto& widgetPtr) {
        return getWidgetAlign(measure, widgetPtr) == Align::center;
    });
    const auto widgetEndIt = ranges::find_if(widgets, [measure](const auto& widgetPtr) {
        return getWidgetAlign(measure, widgetPtr) == Align::end;
    });
    auto centerIndex = std::distance(widgets.begin(), widgetCenterIt);
    auto endIndex = std::distance(widgets.begin(), widgetEndIt);
    centerIndex = std::min(centerIndex, endIndex);

    const auto sizeCenterIt = std::next(widgetSizes.begin(), centerIndex);
    const auto sizeEndIt = std::next(widgetSizes.begin(), endIndex);

    float fullSize = rectSizeProjection(measure, rect);

    float startSize = accumulateMeasure(widgetSizes.begin(), sizeCenterIt, measure);
    float centerSize = accumulateMeasure(sizeCenterIt, sizeEndIt, measure);
    float endSize = accumulateMeasure(sizeEndIt, widgetSizes.end(), measure);

    float cursor = 0.F;
    auto align = Align::start;
    for (const auto& [index, widget] : views::enumerate(widgets)) {
        auto nextAlign = getNextAlign(align, getWidgetAlign(measure, widget));

        cursor = getWidgetCursor(measure, align, nextAlign, centerSize, endSize, rect, cursor);

        auto& widgetSize = widgetSizes.at(static_cast<std::size_t>(index));
        auto currentSize = widgetSizeProjection(measure, widgetSize);
        auto& alignedSize = (nextAlign == Align::start)    ? startSize
                            : (nextAlign == Align::center) ? centerSize
                                                           : endSize;
        alignedSize -= currentSize;
        float availableSize = getAvailableSize(fullSize, startSize, centerSize, endSize, align);

        float cursorX = measure == Measure::horizontal ? cursor
                                                       : rect.x;
        float cursorY = measure == Measure::vertical ? cursor
                                                     : rect.y;
        float width = measure == Measure::horizontal ? widgetSize.width
                                                     : maxElementMeasure(widgetSizes.begin(), widgetSizes.end(),
                                                                         oppositeMeasure(measure));
        float height = measure == Measure::vertical ? widgetSize.height
                                                    : maxElementMeasure(widgetSizes.begin(), widgetSizes.end(),
                                                                        oppositeMeasure(measure));
        float availableWidth = measure == Measure::horizontal ? availableSize
                                                              : rect.width;
        float availableHeight = measure == Measure::vertical ? availableSize
                                                             : rect.height;

        widgetSize = fun(widget,
                         cursorX, cursorY,
                         width, height,
                         availableWidth, availableHeight,
                         widgetSize);
        widgetSize.width = std::min(widgetSize.width, availableWidth);
        widgetSize.height = std::min(widgetSize.height, availableHeight);

        currentSize = widgetSizeProjection(measure, widgetSize);
        alignedSize += currentSize;

        cursor += currentSize + getPadding();
        align = nextAlign;
    }
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
        return fullSize - startSize - endSize;
    case Align::end:
        return (fullSize - centerSize) / 2 - endSize;
    }
    std::unreachable();
}

} // namespace widget
