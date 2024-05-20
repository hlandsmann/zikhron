#include <Separator.h>

#include <utility>

namespace widget {
void Separator::setup(float width, float height)
{
    widgetSize = {.width = width,
                  .height = height};
}

void Separator::setup()
{
    setup(0.F, 0.F);
}

Separator::Separator(WidgetInit init)
    : Widget{std::move(init)}
{}

auto Separator::calculateSize() const -> WidgetSize
{
    return widgetSize;
}

} // namespace widget
