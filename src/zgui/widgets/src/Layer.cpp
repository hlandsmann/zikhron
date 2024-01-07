#include <Layer.h>
#include <detail/MetaBox.h>
#include <detail/Widget.h>

namespace widget {
void Layer::setup()
{
}

Layer::Layer(const WidgetInit& init)
    : MetaBox<Layer>{init}
{}

auto Layer::arrange() -> bool
{
    bool needArrange = false;
    for (const auto& widget : widgets) {
        needArrange |= widget->arrange();
    }
    return needArrange;
}

auto Layer::calculateSize() const -> WidgetSize
{
    return {
            .widthType = expandWidth,
            .heightType = expandHeight,
            .width = max_elementMeasure(widgets.begin(), widgets.end(), Measure::horizontal),
            .height = max_elementMeasure(widgets.begin(), widgets.end(), Measure::vertical),
    };
}
} // namespace widget
