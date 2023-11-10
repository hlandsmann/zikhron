#include "Layout.h"

#include "Widget.h"

#include <algorithm>
#include <memory>
#include <numeric>
#include <ranges>
#include <utility>
#include <vector>

namespace ranges = std::ranges;
namespace views = std::ranges::views;
namespace widget {
Layout::Layout(layout::Orientation _orientation)
    : Layout{std::make_shared<layout::Rect>(), _orientation} {}
Layout::Layout(Align _align, std::shared_ptr<layout::Rect> _rect, layout::Orientation _orientation)
    : Widget<Layout>{_align, _rect}
    , orientation{_orientation}
    , layoutRect{std::move(_rect)} {}
Layout::Layout(std::shared_ptr<layout::Rect> _rect, layout::Orientation _orientation)
    : Widget<Layout>{Align::start, _rect}
    , layoutRect{std::move(_rect)}
    , orientation{_orientation} {}

auto Layout::calculateSize() const -> WidgetSize
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

auto Layout::widgetSizeProjection(const WidgetSize& widgetSize, Measure measure) -> float
{
    switch (measure) {
    case Measure::width:
        return widgetSize.width;
    case Measure::height:
        return widgetSize.height;
    }
    std::unreachable();
}

auto Layout::widgetSizeTypeProjection(const WidgetSize& widgetSize, Measure measure) -> SizeType
{
    switch (measure) {
    case Measure::width:
        return widgetSize.widthType;
    case Measure::height:
        return widgetSize.heightType;
    }
    std::unreachable();
}

auto Layout::rectPositionProjection(layout::Rect& rect, Measure measure) -> float&
{
    switch (measure) {
    case Measure::width:
        return rect.x;
    case Measure::height:
        return rect.y;
    }
    std::unreachable();
}

auto Layout::rectSizeProjection(layout::Rect& rect, Measure measure) -> float&
{
    switch (measure) {
    case Measure::width:
        return rect.width;
    case Measure::height:
        return rect.height;
    }
    std::unreachable();
}

auto Layout::max_elementMeasure(std::vector<std::shared_ptr<WidgetBase>>::const_iterator first,
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

auto Layout::accumulateMeasure(std::vector<std::shared_ptr<WidgetBase>>::const_iterator first,
                               std::vector<std::shared_ptr<WidgetBase>>::const_iterator last,
                               Measure measure) const -> float
{
    return std::accumulate(first, last, float{}, [this, measure](float val, const std::shared_ptr<WidgetBase>& widget) -> float {
        return val + widgetSizeProjection(widget->getWidgetSize(), measure) + ((val == 0.F) ? 0.F : padding);
    });
}

auto Layout::getNextAlign(Align oldAlign, Align nextAlign)
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

auto Layout::getWidgetNewCursor(Align align, float cursor, const WidgetBase& widget,
                                float centerSize, float endSize, Measure measure) const -> float
{
    float widgetSize = widgetSizeProjection(widget.getWidgetSize(), measure);
    switch (align) {
    case Align::start:
        return cursor;
    case Align::center: {
        float rectSize = rectSizeProjection(*layoutRect, measure);
        float centerCursor = rectSize / 2 - widgetSize / 2;
        float minCursor = std::min(rectSize - centerSize, centerCursor);
        return std::max(minCursor, cursor);
    }
    case Align::end:
        float maxCursor = std::max(widgetSize - endSize, cursor);
        if (widgetSizeTypeProjection(widget.getWidgetSize(), measure) == SizeType::fixed
            || widgetSize < endSize) {
            return maxCursor;
        }
        return cursor;
    }
    std::unreachable();
}

auto Layout::getWidgetNewSize(Align align, Align alignNextWidget,
                              float cursor, float cursorNextWidget,
                              const WidgetBase& widget,
                              Measure measure) -> float
{
    float widgetSize = widgetSizeProjection(widget.getWidgetSize(), measure);
    SizeType widgetSizeType = widgetSizeTypeProjection(widget.getWidgetSize(), measure);
    switch (align) {
    case Align::start:
    case Align::center:
        if (align == alignNextWidget || widgetSizeType == SizeType::fixed) {
            return widgetSize;
        }
        return cursorNextWidget - cursor;
    case Align::end:
        if (widgetSizeType == SizeType::fixed) {
            return widgetSize;
        }
        return cursorNextWidget - cursor;
    }
    std::unreachable();
}

void Layout::doLayout()
{
    if (widgets.empty()) {
        return;
    }
    auto measure = Measure::width;

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

    auto& widget = *widgets.front();
    Align align0 = getNextAlign(Align::start, widget.Align());
    Align align1 = align0;
    float cursor0 = getWidgetNewCursor(align0, 0.F, widget, centerSize, endSize, measure);
    float cursor1 = cursor0;
    for (const auto& [widgetPair, widgetRect0] : views::zip(views::pairwise(widgets), rects)) {
        auto& widget0 = *std::get<0>(widgetPair);
        auto& widget1 = *std::get<1>(widgetPair);

        align1 = getNextAlign(align1, widget1.Align()); // old Align is here also align0

        cursor1 = cursor0 + widgetSizeProjection(widget0.getWidgetSize(), measure) + padding;
        cursor1 = getWidgetNewCursor(align1, cursor1, widget1, centerSize, endSize, measure);

        auto size = getWidgetNewSize(align0, align1, cursor0, cursor1, widget0, measure);

        // set values
        rectPositionProjection(*widgetRect0, measure) = cursor0;
        rectSizeProjection(*widgetRect0, measure) = size;

        // only cursor0 needs updating for next loop
        cursor0 = cursor1;
        align0 = align1;
    }
    auto& widgetRect = *rects.back();
    cursor1 = rectSizeProjection(*layoutRect, measure);
    auto size = getWidgetNewSize(align0, align0, cursor0, cursor1, widget, measure);
    rectPositionProjection(widgetRect, measure) = cursor0;
    rectSizeProjection(widgetRect, measure) = size;
}

} // namespace widget
