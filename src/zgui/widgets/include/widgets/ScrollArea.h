#pragma once
#include "Layer.h"
#include "detail/Widget.h" // IWYU pragma: export detail/Widget.h

#include <context/Drop.h>
#include <context/Theme.h>

#include <memory>
#include <string>

namespace widget {

class ScrollAreaDrop;

class ScrollArea : public Widget
{
    using Align = layout::Align;
    using ExpandType = layout::ExpandType;

    template<class T>
    friend class MetaBox;
    friend class Widget;
    friend class Overlay;
    void setup(const std::string& name);

public:
    ScrollArea(const WidgetInit& init);
    ~ScrollArea() override = default;

    ScrollArea(const ScrollArea&) = delete;
    ScrollArea(ScrollArea&&) = delete;
    auto operator=(const ScrollArea&) -> ScrollArea& = delete;
    auto operator=(ScrollArea&&) -> ScrollArea& = delete;

    auto arrange(const layout::Rect& rect) -> bool override;

    [[nodiscard]] auto dropScrollArea() -> ScrollAreaDrop;

    // export Layer functions >>>>>>>>>>>
    template<class WidgetType, class... Args>
    auto add(Align widgetAlign, Args... args) -> std::shared_ptr<WidgetType>
    {
        return layer->add<WidgetType>(widgetAlign, std::forward<Args>(args)...);
    }

    void clear() { layer->clear(); }

    template<class WidgetType>
    auto next() -> WidgetType&
    {
        return layer->next<WidgetType>();
    }

    void start() { layer->start(); }

protected:
    [[nodiscard]] auto getWidgetSizeFromRect(const layout::Rect& rect) -> WidgetSize override;
    auto calculateSize() const -> WidgetSize override;
    auto calculateMinSize() const -> WidgetSize override;

private:
    static constexpr float scrollbarWidth = 8;
    bool scrollbarIsActive{};
    std::shared_ptr<widget::Layer> layer;
    WidgetSize widgetSizeCache{};
};

class ScrollAreaDrop : public context::Drop<ScrollAreaDrop>
{
public:
    ScrollAreaDrop(const std::string& name,
              const widget::layout::Rect& rect,
              context::StyleColorsDrop styleColorsDrop);

private:
    friend class Drop<ScrollAreaDrop>;
    static void pop();
    context::StyleColorsDrop styleColorsDrop;
};
} // namespace widget
