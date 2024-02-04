#include <context/Theme.h>
#include <context/WidgetIdGenerator.h>
#include <detail/Widget.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <memory>
#include <string>
#include <utility>

namespace widget {
Widget::Widget(WidgetInit init)
    : theme{std::move(init.theme)}
    , widgetIdGenerator{std::move(init.widgetIdGenerator)}
    , rectPtr{std::move(init.rect)}
    , horizontalAlign{init.horizontalAlign}
    , verticalAlign{init.verticalAlign}
    , expandTypeWidth{init.expandTypeWidth}
    , expandTypeHeight{init.expandTypeHeight}
    , parent{std::move(init.parent)}
    , widgetId{widgetIdGenerator->getNextId()}
{
}

auto Widget::arrange(const layout::Rect& _rect) -> bool
{
    *rectPtr = _rect;
    return false;
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

auto Widget::getWidgetId() const -> WidgetId
{
    return widgetId;
}

auto Widget::dropWidgetId() const -> context::WidgetIdDrop
{
    return {widgetId};
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

auto Widget::getWidgetSizeFromRect(const layout::Rect& rect) -> WidgetSize
{
    auto widgetSize = getWidgetMinSize();
    if (expandTypeWidth == ExpandType::expand) {
        widgetSize.width = std::max(widgetSize.width, rect.width);
    }
    if (expandTypeHeight == ExpandType::expand) {
        widgetSize.height = std::max(widgetSize.height, rect.height);
    }
    return widgetSize;
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

void Widget::setName(const std::string& _name)
{
    name = _name;
}

auto Widget::getName() const -> const std::string&
{
    if (name.empty()) {
        name = "widget_" + std::to_string(widgetId);
    }
    return name;
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

auto Widget::getRect() const -> const layout::Rect&
{
    return *rectPtr;
}

void Widget::setRect(const layout::Rect& rect)
{
    *rectPtr = rect;
}

void Widget::setExpandType(layout::ExpandType width, layout::ExpandType height)
{
    expandTypeWidth = width;
    expandTypeHeight = height;
}

auto Widget::getExpandTypeWidth() const -> ExpandType
{
    return expandTypeWidth;
}

auto Widget::getExpandTypeHeight() const -> ExpandType
{
    return expandTypeHeight;
}

auto Widget::makeWidgetInit() -> WidgetInit
{
    WidgetInit init = {.theme = theme,
                       .widgetIdGenerator = widgetIdGenerator,
                       .rect = rectPtr,
                       .horizontalAlign = horizontalAlign,
                       .verticalAlign = verticalAlign,
                       .expandTypeWidth = ExpandType::width_fixed,
                       .expandTypeHeight = ExpandType::height_fixed,
                       .parent = shared_from_this()};

    return init;
} // namespace widget

} // namespace widget
