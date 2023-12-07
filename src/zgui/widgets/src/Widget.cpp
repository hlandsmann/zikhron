#include <context/Theme.h>
#include <Widget.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <utility>
namespace widget {
WidgetBase::WidgetBase(WidgetInit init)
    : theme{std::move(init.theme)}
    , rectPtr{std::move(init.rect)}
    , baseOrientation{init.orientation}
    , baseAlign{init.align}
{
}

auto WidgetBase::Rect() const -> const layout::Rect&
{
    return *rectPtr;
}

auto WidgetBase::getTheme() const -> const context::Theme&
{
    return *theme;
}

auto WidgetBase::Orientation() const -> layout::Orientation
{
    return baseOrientation;
}

auto WidgetBase::Align() const -> layout::Align
{
    return baseAlign;
}

auto WidgetBase::getThemePtr() const -> std::shared_ptr<context::Theme>
{
    return theme;
}

} // namespace widget
