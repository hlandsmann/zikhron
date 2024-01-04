#pragma once
#include "Widget.h"

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>
namespace widget {
template<class BoxImpl>
class MetaBox : public Widget
{
    using Align = layout::Align;
    using Orientation = layout::Orientation;
    using SizeType = layout::SizeType;

public:
    MetaBox(const WidgetInit& init);

    void setPadding(float padding);
    void setBorder(float border);

    template<class WidgetType, class... Args>
    auto add(Align widgetAlign, Args... args) -> std::shared_ptr<WidgetType>
    {
        auto* self = static_cast<BoxImpl*>(this);
        auto widgetRect = std::make_shared<layout::Rect>();
        WidgetInit init = {.theme = getThemePtr(),
                           .widgetIdGenerator = getWidgetIdGenerator(),
                           .rect = widgetRect,
                           .orientation = self->getChildOrientation(),
                           .align = widgetAlign,
                           .parent = shared_from_this()};
        auto widget = std::make_shared<WidgetType>(std::move(init));
        widget->setup(std::forward<Args>(args)...);
        if constexpr (std::is_same_v<WidgetType, BoxImpl>) {
            widget->setPadding(padding);
        }
        self->widgets.push_back(static_cast<std::shared_ptr<Widget>>(widget));
        self->rects.push_back(std::move(widgetRect));

        resetWidgetSize();
        setArrangeIsNecessary();
        return widget;
    }
    void pop();
    void clear();

    template<class WidgetType>
    auto next() -> WidgetType&
    {
        auto* self = static_cast<BoxImpl*>(this);
        if (!(currentWidgetIt < self->widgets.end())) {
            throw std::out_of_range("Bad call of next(). Forgot to call start()?");
        }
        auto& result = dynamic_cast<WidgetType&>(**currentWidgetIt);
        currentWidgetIt++;
        return result;
    }
    [[nodiscard]] auto isLast() const -> bool;
    void start();
    auto numberOfWidgets() const -> std::size_t;

    constexpr static float s_padding = 16.F;

protected:
    auto getBorderedRect() const -> layout::Rect;
    auto getBorder() const -> float;
    auto getPadding() const -> float;

private:
    float padding{s_padding};
    float border{};
    std::vector<std::shared_ptr<Widget>>::iterator currentWidgetIt;
};

} // namespace widget
