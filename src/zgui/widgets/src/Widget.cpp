#include <Widget.h>
#include <context/Theme.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <utility>
namespace widget {
WidgetBase::WidgetBase(WidgetInit init)
    : theme{std::move(init.theme)}
    , rectPtr{std::move(init.rect)}
    , passiveOrientation{init.orientation}
    , baseAlign{init.align}
    , parent{std::move(init.parent)}
{
}

auto WidgetBase::arrangeIsNecessary() -> bool
{
    bool tmpArrangeNecessary = arrangeNecessary;
    arrangeNecessary = false;
    return tmpArrangeNecessary;
}

void WidgetBase::setArrangeIsNecessary()
{
    if (auto parentPtr = parent.lock()) {
        parentPtr->setArrangeIsNecessary();
    } else {
        arrangeNecessary = true;
    }
}

auto WidgetBase::getTheme() const -> const context::Theme&
{
    return *theme;
}

auto WidgetBase::PassiveOrientation() const -> layout::Orientation
{
    return passiveOrientation;
}

auto WidgetBase::Align() const -> layout::Align
{
    return baseAlign;
}

auto WidgetBase::getThemePtr() const -> std::shared_ptr<context::Theme>
{
    return theme;
}

auto WidgetBase::Rect() const -> const layout::Rect&
{
    return *rectPtr;
}

} // namespace widget
