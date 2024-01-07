#include <context/Theme.h>
#include <context/WidgetIdGenerator.h>
#include <detail/Widget.h>
#include <spdlog/spdlog.h>

#include <cstddef>
#include <memory>
#include <utility>
namespace widget {
Widget::Widget(WidgetInit init)
    : theme{std::move(init.theme)}
    , widgetIdGenerator{std::move(init.widgetIdGenerator)}
    , rectPtr{std::move(init.rect)}
    , passiveOrientation{init.orientation}
    , horizontalAlign{init.horizontalAlign}
    , verticalAlign{init.verticalAlign}
    , parent{std::move(init.parent)}
    , widgetId{widgetIdGenerator->getNextId()}
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

auto Widget::getWidgetId() const -> int
{
    return widgetId;
}

auto Widget::dropWidgetId() const -> context::WidgetIdDrop
{
    return {widgetId};
}

auto Widget::PassiveOrientation() const -> layout::Orientation
{
    return passiveOrientation;
}

auto Widget::HorizontalAlign() const -> layout::Align
{
    return horizontalAlign;
}

void Widget::setHorizontalAlign(layout::Align align)
{
    horizontalAlign = align;
    setArrangeIsNecessary();
}

auto Widget::VerticalAlign() const -> layout::Align
{
    return verticalAlign;
}

void Widget::setVerticalAlign(layout::Align align)
{
    verticalAlign = align;
    setArrangeIsNecessary();
}

auto Widget::getWidgetSize() const -> const WidgetSize&
{
    if (optWidgetSize.has_value()) {
        return *optWidgetSize;
    }
    return optWidgetSize.emplace(calculateSize());
}

auto Widget::getWidgetMinSize() const -> const WidgetSize&
{
    if (optWidgetMinSize.has_value()) {
        return *optWidgetMinSize;
    }
    return optWidgetMinSize.emplace(calculateMinSize());
}

void Widget::resetWidgetSize()
{
    if (auto parentPtr = parent.lock()) {
        parentPtr->resetWidgetSize();
    }
    optWidgetSize.reset();
    optWidgetMinSize.reset();
}

auto Widget::calculateMinSize() const -> WidgetSize
{
    return calculateSize();
}

auto Widget::getThemePtr() const -> std::shared_ptr<context::Theme>
{
    return theme;
}

auto Widget::getWidgetIdGenerator() const -> std::shared_ptr<context::WidgetIdGenerator>
{
    return widgetIdGenerator;
}

auto Widget::Rect() const -> const layout::Rect&
{
    return *rectPtr;
}

auto Widget::getRectPtr() const -> std::shared_ptr<layout::Rect>
{
    return rectPtr;
}

auto Widget::makeWidgetInit() -> WidgetInit
{
    WidgetInit init = {.theme = theme,
                       .widgetIdGenerator = widgetIdGenerator,
                       .rect = rectPtr,
                       .orientation = passiveOrientation,
                       .horizontalAlign = horizontalAlign,
                       .parent = shared_from_this()};
    return init;
}

} // namespace widget
