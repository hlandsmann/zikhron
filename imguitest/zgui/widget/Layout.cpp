#include "Layout.h"

#include "Widget.h"

#include <algorithm>
#include <memory>
#include <numeric>
#include <utility>
#include <vector>

namespace ranges = std::ranges;
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
                .widthType = size_type::variable,
                .heightType = size_type::fixed,
                .width = accumulateMeasure(widgets.begin(), widgets.end(), Measure::width),
                .height = max_elementMeasure(widgets.begin(), widgets.end(), Measure::height),
        };
        break;
    case Orientation::vertical:
        result = {
                .widthType = size_type::variable,
                .heightType = size_type::fixed,
                .width = max_elementMeasure(widgets.begin(), widgets.end(), Measure::width),
                .height = accumulateMeasure(widgets.begin(), widgets.end(), Measure::height)};

        break;
    }
    return result;
}

auto Layout::measureProjection(const std::shared_ptr<WidgetBase>& widget, Measure measure) -> float
{
    switch (measure) {
    case Measure::width:
        return widget->LayoutSize().width;
    case Measure::height:
        return widget->LayoutSize().height;
    }
}

auto Layout::max_elementMeasure(std::vector<std::shared_ptr<WidgetBase>>::const_iterator first,
                                std::vector<std::shared_ptr<WidgetBase>>::const_iterator last,
                                Measure measure) -> float
{
    if (first == last) {
        return 0.0F;
    }
    return measureProjection(
            *std::max_element(first, last,
                              [measure](const std::shared_ptr<WidgetBase>& widget_a,
                                        const std::shared_ptr<WidgetBase>& widget_b) {
                                  return measureProjection(widget_a, measure) < measureProjection(widget_b, measure);
                              }),
            measure);
}

auto Layout::accumulateMeasure(std::vector<std::shared_ptr<WidgetBase>>::const_iterator first,
                               std::vector<std::shared_ptr<WidgetBase>>::const_iterator last,
                               Measure measure) const -> float
{
    return std::accumulate(first, last, float{}, [this, measure](float val, const std::shared_ptr<WidgetBase>& widget) -> float {
        return val + measureProjection(widget, measure) + ((val == 0.F) ? 0.F : padding);
    });
}

void Layout::doLayout()
{
    // auto align = [](const std::shared_ptr<WidgetBase>& widget) -> layout::Align{
    //   return widget->LayoutSize().

    std::vector<float> start;
    std::vector<float> center;
    std::vector<float> end;
    // float x_min = size->x;
    using layout::Align;
    Align lastAlign = Align::start;
    for (const auto& widget : widgets) {
    }
}
} // namespace widget
