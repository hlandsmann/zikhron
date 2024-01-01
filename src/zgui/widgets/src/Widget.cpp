#include <Widget.h>
#include <context/Theme.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <utility>
namespace widget {
Widget::Widget(WidgetInit init)
    : theme{std::move(init.theme)}
    , rectPtr{std::move(init.rect)}
    , passiveOrientation{init.orientation}
    , baseAlign{init.align}
    , parent{std::move(init.parent)}
{
}

auto Widget::arrangeIsNecessary() -> bool
{
    bool tmpArrangeNecessary = arrangeNecessary;
    arrangeNecessary = false;
    return tmpArrangeNecessary;
}

void Widget::setArrangeIsNecessary()
{
    if (auto parentPtr = parent.lock()) {
        parentPtr->setArrangeIsNecessary();
    } else {
        arrangeNecessary = true;
    }
}

auto Widget::getTheme() const -> const context::Theme&
{
    return *theme;
}

auto Widget::PassiveOrientation() const -> layout::Orientation
{
    return passiveOrientation;
}

auto Widget::Align() const -> layout::Align
{
    return baseAlign;
}

auto Widget::getWidgetSize() const -> const WidgetSize&
{
    if (optWidgetSize.has_value()) {
        return *optWidgetSize;
    }
    return optWidgetSize.emplace(calculateSize());
}

void Widget::resetWidgetSize()
{
    if (auto parentPtr = parent.lock()) {
        parentPtr->resetWidgetSize();
    }
    optWidgetSize.reset();
}

auto Widget::getThemePtr() const -> std::shared_ptr<context::Theme>
{
    return theme;
}

auto Widget::Rect() const -> const layout::Rect&
{
    return *rectPtr;
}

auto Widget::getRectPtr() const -> std::shared_ptr<layout::Rect>
{
    return rectPtr;
}

} // namespace widget
