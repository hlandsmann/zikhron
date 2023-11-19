#include <Box.h>
#include <Widget.h>
#include <context/Theme.h>
#include <context/imglog.h>
#include <spdlog/spdlog.h>

#include <algorithm>
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
Box::Box(std::shared_ptr<Theme> _theme, layout::Orientation _orientation)
    : Box{std::move(_theme), _orientation, std::make_shared<layout::Rect>()} {}
Box::Box(std::shared_ptr<Theme> _theme, layout::Orientation _orientation, Align _align, std::shared_ptr<layout::Rect> _rect)
    : Widget<Box>{std::move(_theme), _orientation, _align, _rect}
    , orientation{_orientation}
    , layoutRect{std::move(_rect)} {}
Box::Box(std::shared_ptr<Theme> _theme, layout::Orientation _orientation, std::shared_ptr<layout::Rect> _rect)
    : Widget<Box>{std::move(_theme), _orientation, Align::start, _rect}
    , orientation{_orientation}
    , layoutRect{std::move(_rect)}
{}

void Box::arrange(const layout::Rect& rect)
{
    *layoutRect = rect;
    arrange();
}

void Box::arrange()
{
    if (orientation == layout::Orientation::horizontal) {
        doLayout(Measure::width);
    } else {
        doLayout(Measure::height);
    }
    start();
}

void Box::start()
{
    currentWidgetIt = widgets.begin();
}

auto Box::calculateSize() const -> WidgetSize
{
    WidgetSize result{};
    if (widgets.empty()) {
        return result;
    }
    using layout::Orientation;
    switch (orientation) {
    case Orientation::horizontal:
        result = {
                .widthType = SizeType::expand,
                .heightType = SizeType::fixed,
                .width = accumulateMeasure(widgets.begin(), widgets.end(), Measure::width),
                .height = max_elementMeasure(widgets.begin(), widgets.end(), Measure::height),
        };
        break;
    case Orientation::vertical:
        result = {
                .widthType = SizeType::expand,
                .heightType = SizeType::fixed,
                .width = max_elementMeasure(widgets.begin(), widgets.end(), Measure::width),
                .height = accumulateMeasure(widgets.begin(), widgets.end(), Measure::height)};

        break;
    }
    return result;
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

auto Box::max_elementMeasure(std::vector<std::shared_ptr<WidgetBase>>::const_iterator first,
                             std::vector<std::shared_ptr<WidgetBase>>::const_iterator last,
                             Measure measure) -> float
{
    if (first == last) {
        return 0.0F;
    }
    return widgetSizeProjection(
            (*std::max_element(first, last,
                               [measure](const std::shared_ptr<WidgetBase>& widget_a,
                                         const std::shared_ptr<WidgetBase>& widget_b) -> bool {
                                   return widgetSizeProjection(widget_a->getWidgetSize(), measure)
                                          < widgetSizeProjection(widget_b->getWidgetSize(), measure);
                               }))
                    ->getWidgetSize(),
            measure);
}

auto Box::accumulateMeasure(std::vector<std::shared_ptr<WidgetBase>>::const_iterator first,
                            std::vector<std::shared_ptr<WidgetBase>>::const_iterator last,
                            Measure measure) const -> float
{
    return std::accumulate(first, last, float{}, [this, measure](float val, const std::shared_ptr<WidgetBase>& widget) -> float {
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

auto Box::getWidgetNewCursor(Align align, float cursor, const WidgetBase& widget,
                             float centerSize, float endSize, Measure measure) const -> float
{
    float rectSize = rectSizeProjection(*layoutRect, measure);
    float widgetSize = widgetSizeProjection(widget.getWidgetSize(), measure);
    // imglog::log("wsize: {}, align {}", widgetSize, static_cast<int>(align));
    switch (align) {
    case Align::start:
        // imglog::log("StartCursor: {}", cursor);
        return cursor;
    case Align::center: {
        float centerCursor = rectSize / 2 - widgetSize / 2;
        float minCursor = std::min(rectSize - centerSize, centerCursor);
        return std::max(minCursor, cursor);
    }
    case Align::end:
        // imglog::log("EndCursor: {}", cursor);
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
                           const WidgetBase& widget,
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
        rect->x = layoutRect->x;
        rect->y = layoutRect->y;
        auto widgetSize = widget->getWidgetSize();
        rect->width = widgetSize.widthType == SizeType::fixed ? widgetSize.width : layoutRect->width;
        rect->height = widgetSize.heightType == SizeType::fixed ? widgetSize.height : layoutRect->height;
    }
}

void Box::doLayout(Measure measure)
{
    imglog::log("layout x: {}, y: {}, w: {}, h: {}", layoutRect->x, layoutRect->y, layoutRect->width, layoutRect->height);
    if (widgets.empty()) {
        return;
    }
    // static int i = 0;
    // if (i < 10) {
    //     i++;
    //     spdlog::warn("b: {}, f: {}",
    //                  static_cast<int>(widgets.back()->Align()),
    //                  static_cast<int>(widgets.front()->Align()));
    // }
    setChildWidgetsInitialRect();

    // for (const auto& widget : widgets) {
    //     imglog::log("waling: {}", static_cast<int>(widget->Align()));
    // }
    auto centerIt = ranges::find_if(widgets, [](const auto& widgetPtr) {
        return widgetPtr->Align() == Align::center
               || widgetPtr->Align() == Align::end;
    });
    auto endIt = ranges::find_if(widgets, [](const auto& widgetPtr) {
        return widgetPtr->Align() == Align::end;
    });
    // float startSize = accumulateMeasure(widgets.begin(), centerIt, measure);
    float centerSize = accumulateMeasure(centerIt, widgets.end(), measure);
    float endSize = accumulateMeasure(endIt, widgets.end(), measure);
    auto widget0 = widgets.front();
    Align align0 = getNextAlign(Align::start, widget0->Align());
    Align align1 = align0;
    float cursor0 = getWidgetNewCursor(align0, rectPositionProjection(*layoutRect, measure), *widget0, centerSize, endSize, measure);
    float cursor1 = cursor0;
    for (const auto& [widget1, widgetRect0] : views::zip(std::span{std::next(widgets.begin()), widgets.end()}, rects)) {
        // auto& widget0 = *std::get<0>(widgetPair);
        // auto& widget1 = *std::get<1>(widgetPair);

        align1 = getNextAlign(align1, widget1->Align()); // old Align is here also align0

        cursor1 = cursor0 + widgetSizeProjection(widget0->getWidgetSize(), measure) + padding;
        cursor1 = getWidgetNewCursor(align1, cursor1, *widget1, centerSize, endSize, measure);

        auto size = getWidgetNewSize(align0, align1, cursor0, cursor1, *widget0, measure);

        // set values
        // imglog::log("SetCursor {}", cursor0);
        // imglog::log("SetSize {}", size);
        rectPositionProjection(*widgetRect0, measure) = cursor0;
        rectSizeProjection(*widgetRect0, measure) = size;

        // only cursor0 needs updating for next loop
        cursor0 = cursor1;
        align0 = align1;
        widget0 = widget1;
    }
    // if (i < 10) {
    //     i++;
    //     spdlog::warn("b: {}, f: {}",
    //                  static_cast<int>(widgets.back()->Align()),
    //                  static_cast<int>(widgets.front()->Align()));
    // }
    auto& widgetRect = *rects.back();
    cursor1 = rectSizeProjection(*layoutRect, measure);
    auto size = getWidgetNewSize(align0, align0, cursor0, cursor1, *widget0, measure);

    // imglog::log("SetCursor {}", cursor0);
    // imglog::log("SetSize {}", size);
    rectPositionProjection(widgetRect, measure) = cursor0;
    rectSizeProjection(widgetRect, measure) = size;
    // if (i < 10) {
    //     i++;
    //     spdlog::warn("b: {}, f: {}",
    //                  static_cast<int>(widgets.back()->Align()),
    //                  static_cast<int>(widgets.front()->Align()));
    // }
}

} // namespace widget
