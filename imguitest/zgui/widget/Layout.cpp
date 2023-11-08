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
Layout::Layout(layout::Align _align, std::shared_ptr<layout::Rect> _rect, layout::Orientation _orientation)
    : Widget<Layout>{_align, _rect}
    , orientation{_orientation}
    , rect{std::move(_rect)} {}
Layout::Layout(std::shared_ptr<layout::Rect> _rect, layout::Orientation _orientation)
    : Widget<Layout>{layout::Align::start, _rect}
    , rect{std::move(_rect)}
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

auto Layout::widgetSizeProjection(const std::shared_ptr<WidgetBase>& widget, Measure measure) -> float
{
    switch (measure) {
    case Measure::width:
        return widget->getWidgetSize().width;
    case Measure::height:
        return widget->getWidgetSize().height;
    }
    std::unreachable();
}

auto Layout::rectPositionProjection(const std::shared_ptr<layout::Rect>& rect, Measure measure) -> float&
{
    switch (measure) {
    case Measure::width:
        return rect->x;
    case Measure::height:
        return rect->y;
    }
    std::unreachable();
}

auto Layout::rectSizeProjection(const std::shared_ptr<layout::Rect>& rect, Measure measure) -> float&
{
    switch (measure) {
    case Measure::width:
        return rect->width;
    case Measure::height:
        return rect->height;
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
            *std::max_element(first, last,
                              [measure](const std::shared_ptr<WidgetBase>& widget_a,
                                        const std::shared_ptr<WidgetBase>& widget_b) {
                                  return widgetSizeProjection(widget_a, measure) < widgetSizeProjection(widget_b, measure);
                              }),
            measure);
}

auto Layout::accumulateMeasure(std::vector<std::shared_ptr<WidgetBase>>::const_iterator first,
                               std::vector<std::shared_ptr<WidgetBase>>::const_iterator last,
                               Measure measure) const -> float
{
    return std::accumulate(first, last, float{}, [this, measure](float val, const std::shared_ptr<WidgetBase>& widget) -> float {
        return val + widgetSizeProjection(widget, measure) + ((val == 0.F) ? 0.F : padding);
    });
}

auto Layout::getNextAlign(layout::Align oldAlign, layout::Align nextAlign)
{
    using layout::Align;
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
    default:
        return Align::end;
    }
}

void Layout::doLayout()
{
    auto measure = Measure::width;

    auto centerIt = ranges::find_if(widgets, [](const auto& widgetPtr) {
        return widgetPtr->Align() == layout::Align::center
               || widgetPtr->Align() == layout::Align::end;
    });
    auto endIt = ranges::find_if(widgets, [](const auto& widgetPtr) {
        return widgetPtr->Align() == layout::Align::end;
    });
    float startSize = accumulateMeasure(widgets.begin(), centerIt, measure);
    float centerSize = accumulateMeasure(centerIt, endIt, measure);
    float endSize = accumulateMeasure(endIt, widgets.end(), measure);

    using layout::Align;
    Align align = Align::start;
    auto size_type = SizeType::fixed;
    float cursor{};
    for (const auto& [widgetPair, widgetRect0] : views::zip(views::pairwise(widgets), rects)) {
        auto& widget0 = *std::get<0>(widgetPair);
        auto& widget1 = *std::get<1>(widgetPair);

        // align = getNextAlign(align, widget->Align());
    }
}

} // namespace widget
