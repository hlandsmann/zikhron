#include <context/Theme.h>
#include <context/WidgetId.h>
#include <detail/Widget.h>
#include <spdlog/spdlog.h>
#include <utils/format.h>

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

auto Widget::dropWidgetId(WidgetId _widgetId) -> context::WidgetIdDrop
{
    return {_widgetId};
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
        widgetSize.width = rect.width;
    }
    if (expandTypeHeight == ExpandType::expand) {
        widgetSize.height = rect.height;
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

auto Widget::getWidgetIdName() const -> const std::string&
{
    if (widgetIdName.empty()) {
        widgetIdName = getName() + "##" + std::to_string(widgetId);
    }
    return widgetIdName;
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

auto Widget::getParent() const -> std::shared_ptr<Widget>
{
    return parent.lock();
}

auto Widget::anyParentHasName(const std::string& _name) const -> bool
{
    Widget const* widget = this;
    while (widget != nullptr) {
        if (widget->getName() == _name) {
            return true;
        }
        widget = widget->getParent().get();
    }
    return false;
}

auto Widget::anyParentHasId(WidgetId _id) const -> bool
{
    Widget const* widget = this;
    while (widget != nullptr) {
        if (widget->getWidgetId() == _id) {
            return true;
        }
        widget = widget->getParent().get();
    }
    return false;
}

void Widget::scratchDbg()
{
    Widget const* widget = this;
    while (widget != nullptr) {
        imglog::log("id is: {}, name {}:", static_cast<unsigned>(widget->getWidgetId()), widget->getName());
        widget = widget->getParent().get();
    }
}

void Widget::setRect(const layout::Rect& rect)
{
    *rectPtr = rect;
}

void Widget::setLocalOffset(float x, float y)
{
    localOffset.x = x;
    localOffset.y = y;
}

auto Widget::getLocalOffset() const -> const layout::Rect&
{
    return localOffset;
}

auto Widget::getOffsetRect() const -> layout::Rect
{
    auto widgetParent = getParent();
    while (widgetParent) {
        const auto& parentOffset = widgetParent->getLocalOffset();
        if (parentOffset.x != 0 || parentOffset.y != 0) {
            return parentOffset;
        }
        widgetParent = widgetParent->getParent();
    }
    return {};
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

void Widget::cutWidgetIdGen()
{
    widgetIdGenerator = std::make_shared<context::WidgetIdGenerator>();
    widgetId = widgetIdGenerator->getNextId();
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
