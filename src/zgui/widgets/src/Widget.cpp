#include <context/Theme.h>
#include <Widget.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <utility>
namespace widget {
WidgetBase::WidgetBase(std::shared_ptr<Theme> _theme,
                       layout::Orientation _orientation,
                       layout::Align _align,
                       std::shared_ptr<layout::Rect> _rect)
    : theme{std::move(_theme)}
    , baseOrientation{_orientation}
    , baseAlign{_align}
    , rectPtr{std::move(_rect)}
{
}

auto WidgetBase::Rect() const -> const layout::Rect&
{
    return *rectPtr;
}

auto WidgetBase::getTheme() const -> const Theme&
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

auto WidgetBase::getThemePtr() const -> std::shared_ptr<Theme>
{
    return theme;
}

} // namespace widget
