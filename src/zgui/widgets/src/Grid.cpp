#include <Grid.h>
#include <detail/MetaBox.h>
#include <detail/Widget.h>

#include <cstddef>
#include <memory>

namespace widget {

void Grid::setup(std::size_t _rows)
{
    rows = _rows;
}

Grid::Grid(const WidgetInit& init)
    : MetaBox<Grid>{init}
{}

auto Grid::arrange(const layout::Rect& rect) -> bool
{
    bool needArrange = false;
    for (const auto& widget : widgets) {
        needArrange |= widget->arrange(rect);
    }
    return needArrange;
}

auto Grid::calculateSize() const -> WidgetSize
{
    return {};
}
auto Grid::calculateMinSize() const -> WidgetSize
{
    return {};
}
} // namespace widget
