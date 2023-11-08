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

auto Layout::sizeProjection(const std::shared_ptr<WidgetBase>& widget, Measure measure) -> float
{
    switch (measure) {
    case Measure::width:
        return widget->LayoutSize().width;
    case Measure::height:
        return widget->LayoutSize().height;
    }
    std::unreachable();
}

auto Layout::positionProjection(const std::shared_ptr<layout::Rect>& rect, Measure measure) -> float&
{
    switch (measure) {
    case Measure::width:
        return rect->x;
    case Measure::height:
        return rect->y;
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
    return sizeProjection(
            *std::max_element(first, last,
                              [measure](const std::shared_ptr<WidgetBase>& widget_a,
                                        const std::shared_ptr<WidgetBase>& widget_b) {
                                  return sizeProjection(widget_a, measure) < sizeProjection(widget_b, measure);
                              }),
            measure);
}

auto Layout::accumulateMeasure(std::vector<std::shared_ptr<WidgetBase>>::const_iterator first,
                               std::vector<std::shared_ptr<WidgetBase>>::const_iterator last,
                               Measure measure) const -> float
{
    return std::accumulate(first, last, float{}, [this, measure](float val, const std::shared_ptr<WidgetBase>& widget) -> float {
        return val + sizeProjection(widget, measure) + ((val == 0.F) ? 0.F : padding);
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
    float startSize{};
    float centerSize{};
    float endSize{};

    using layout::Align;
    Align align = Align::start;
    for (const auto& widget : widgets) {
        align = getNextAlign(align, widget->Align());
        switch (align) {
        case Align::start:
            startSize += sizeProjection(widget, measure);
            break;
        case Align::center:
            centerSize += sizeProjection(widget, measure);
            break;
        case Align::end:
            endSize += sizeProjection(widget, measure);
            break;
        }
    }
    auto size_type = SizeType::fixed;
    align = Align::start;
    float cursor{};
    for (const auto& [widget, wRect] : views::zip(widgets, rects)) {
        align = getNextAlign(align, widget->Align());
    }
}

} // namespace widget
