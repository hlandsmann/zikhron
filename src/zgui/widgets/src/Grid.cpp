#include <Grid.h>
#include <memory>
#include <detail/Widget.h>

namespace widget {

Grid::Grid(const WidgetInit& init)
    : Widget{init}
    , borderedRect{std::make_shared<layout::Rect>()}
{}

auto Grid::arrange() -> bool
{
    bool needArrange = false;
    for (const auto& widget : widgets) {
        needArrange |= widget->arrange();
    }
    return needArrange;
}

void Grid::setPadding(float _padding)
{
    padding = _padding;
}

void Grid::setBorder(float _border)
{
    border = _border;
    const auto& rect = Rect();
    borderedRect->x = rect.x + border;
    borderedRect->y = rect.y + border;
    borderedRect->width = rect.width - border * 2;
    borderedRect->height = rect.height - border * 2;
}

void Grid::start()
{
    currentWidgetIt = widgets.begin();
}

}
