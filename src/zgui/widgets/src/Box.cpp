#include <Box.h>
#include <context/Theme.h>
#include <context/imglog.h>
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
    // , expandWidth{orientation == layout::Orientation::horizontal ? layout::SizeType::width_expand
    //                                                              : layout::SizeType::width_fixed}
    // , expandHeight{orientation == layout::Orientation::vertical ? layout::SizeType::height_expand
    //                                                             : layout::SizeType::height_fixed}
    , borderedRect{std::make_shared<layout::Rect>()}
{}

auto Box::arrange() -> bool
{
    setBorder(border);
    if (orientation == layout::Orientation::horizontal) {
        doLayout(Measure::width);
    } else {
        doLayout(Measure::height);
    }
    bool needArrange = false;
    for (const auto& widget : widgets) {
        needArrange |= widget->arrange();
    }
    return needArrange;
}

void Box::setBorder(float _border)
{
    border = _border;
    const auto& rect = Rect();
    borderedRect->x = rect.x + border;
    borderedRect->y = rect.y + border;
    borderedRect->width = rect.width - border * 2;
    borderedRect->height = rect.height - border * 2;
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

void Box::setPadding(float _padding)
{
    padding = _padding;
}

auto Box::getExpandedSize() const -> WidgetSize
{
    auto rect = *borderedRect;
    auto widgetSize = getWidgetSize();
    widgetSize.width = rect.width;
    widgetSize.height = rect.height;
    return widgetSize;
}


auto Box::calculateSize() const -> WidgetSize
{
    WidgetSize result{};
    using layout::Orientation;
    switch (orientation) {
    case Orientation::horizontal:
        result = {
                .widthType = expandWidth,
                .heightType = expandHeight,
                .width = accumulateMeasure(widgets.begin(), widgets.end(), Measure::width),
                .height = max_elementMeasure(widgets.begin(), widgets.end(), Measure::height),
        };
        break;
    case Orientation::vertical:
        result = {
                .widthType = expandWidth,
                .heightType = expandHeight,
                .width = max_elementMeasure(widgets.begin(), widgets.end(), Measure::width),
                .height = accumulateMeasure(widgets.begin(), widgets.end(), Measure::height)};

        break;
    }
    result.width += border * 2;
    result.height += border * 2;
    return result;
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

auto Box::widgetSizeProjection(const WidgetSize& widgetSize, Measure measure) -> float
{
    switch (measure) {
    case Measure::width:
        return widgetSize.width;
    case Measure::height:
        return widgetSize.height;
    }
    std::unreachable();
}

auto Box::widgetSizeTypeProjection(const WidgetSize& widgetSize, Measure measure) -> SizeType
{
    switch (measure) {
    case Measure::width:
        return widgetSize.widthType;
    case Measure::height:
        return widgetSize.heightType;
    }
    std::unreachable();
}

auto Box::rectPositionProjection(layout::Rect& rect, Measure measure) -> float&
{
    switch (measure) {
    case Measure::width:
        return rect.x;
    case Measure::height:
        return rect.y;
    }
    std::unreachable();
}

auto Box::rectSizeProjection(layout::Rect& rect, Measure measure) -> float&
{
    switch (measure) {
    case Measure::width:
        return rect.width;
    case Measure::height:
        return rect.height;
    }
    std::unreachable();
}

auto Box::max_elementMeasure(std::vector<std::shared_ptr<Widget>>::const_iterator first,
                             std::vector<std::shared_ptr<Widget>>::const_iterator last,
                             Measure measure)
        -> float
{
    if (first == last) {
        return 0.0F;
    }
    return widgetSizeProjection(
            (*std::max_element(first, last,
                               [measure](const std::shared_ptr<Widget>& widget_a,
                                         const std::shared_ptr<Widget>& widget_b) -> bool {
                                   return widgetSizeProjection(widget_a->getWidgetSize(), measure)
                                          < widgetSizeProjection(widget_b->getWidgetSize(), measure);
                               }))
                    ->getWidgetSize(),
            measure);
}

auto Box::accumulateMeasure(std::vector<std::shared_ptr<Widget>>::const_iterator first,
                            std::vector<std::shared_ptr<Widget>>::const_iterator last,
                            Measure measure) const -> float
{
    return std::accumulate(first, last, float{}, [this, measure](float val, const std::shared_ptr<Widget>& widget) -> float {
        return val + widgetSizeProjection(widget->getWidgetSize(), measure) + ((val == 0.F) ? 0.F : padding);
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
    float rectSize = rectSizeProjection(*borderedRect, measure);
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
        if (widgetSizeTypeProjection(widget.getWidgetSize(), measure) == SizeType::fixed
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
    SizeType projectedSizeType = widgetSizeTypeProjection(widget.getWidgetSize(), measure);
    switch (align) {
    case Align::start:
    case Align::center:
        if (align == alignNextWidget || projectedSizeType == SizeType::fixed) {
            return projectedSize;
        }
        return cursorNextWidget - cursor - padding;
    case Align::end:
        if (projectedSizeType == SizeType::fixed) {
            return projectedSize;
        }
        return cursorNextWidget - cursor;
    }
    std::unreachable();
}

void Box::setChildWidgetsInitialRect()
{
    for (const auto& [widget, rect] : views::zip(widgets, rects)) {
        rect->x = borderedRect->x;
        rect->y = borderedRect->y;
        auto widgetSize = widget->getWidgetSize();
        rect->width = widgetSize.widthType == SizeType::fixed ? widgetSize.width : borderedRect->width;
        rect->height = widgetSize.heightType == SizeType::fixed ? widgetSize.height : borderedRect->height;
    }
}

void Box::doLayout(Measure measure)
{
    if (widgets.empty()) {
        return;
    }
    setChildWidgetsInitialRect();

    auto centerIt = ranges::find_if(widgets, [](const auto& widgetPtr) {
        return widgetPtr->Align() == Align::center
               || widgetPtr->Align() == Align::end;
    });
    auto endIt = ranges::find_if(widgets, [](const auto& widgetPtr) {
        return widgetPtr->Align() == Align::end;
    });
    float centerSize = accumulateMeasure(centerIt, widgets.end(), measure);
    float endSize = accumulateMeasure(endIt, widgets.end(), measure);
    auto widget0 = widgets.front();
    Align align0 = getNextAlign(Align::start, widget0->Align());
    Align align1 = align0;
    float cursor0 = getWidgetNewCursor(align0,
                                       rectPositionProjection(*borderedRect, measure),
                                       *widget0,
                                       centerSize, endSize,
                                       measure);

    float cursor1 = cursor0;
    for (const auto& [widget1, widgetRect0] : views::zip(std::span{std::next(widgets.begin()), widgets.end()}, rects)) {
        align1 = getNextAlign(align1, widget1->Align()); // old Align is here also align0

        cursor1 = cursor0 + widgetSizeProjection(widget0->getWidgetSize(), measure) + padding;
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
    cursor1 = rectSizeProjection(*borderedRect, measure);
    auto size = getWidgetNewSize(align0, align0, cursor0, cursor1, *widget0, measure);

    rectPositionProjection(widgetRect, measure) = cursor0;
    rectSizeProjection(widgetRect, measure) = size;
}

} // namespace widget
