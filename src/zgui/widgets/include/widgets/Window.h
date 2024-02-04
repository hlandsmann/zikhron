#pragma once
#include "Box.h"
#include "Grid.h"
#include "Layer.h"
#include "detail/Widget.h"

#include <context/Drop.h>
#include <context/Theme.h>
#include <imgui.h>

#include <memory>
#include <string>
#include <type_traits>

namespace widget {

class WindowDrop;

class Window : public Widget
{
    using Align = layout::Align;
    using ExpandType = layout::ExpandType;

public:
    void setup(ExpandType expandTypeWidth,
               ExpandType expandTypeHeight,
               std::string name);
    Window(const WidgetInit& init);
    ~Window() override = default;

    Window(const Window&) = default;
    Window(Window&&) = default;
    auto operator=(const Window&) -> Window& = default;
    auto operator=(Window&&) -> Window& = default;

    auto arrange(const layout::Rect& rect) -> bool override;

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

    [[nodiscard]] auto dropWindow() -> WindowDrop;

protected:
    auto calculateSize() const -> WidgetSize override;
    auto calculateMinSize() const -> WidgetSize override;

private:
    std::shared_ptr<widget::Layer> layer;
    ExpandType expandTypeWidth{ExpandType::fixed};
    ExpandType expandTypeHeight{ExpandType::fixed};
};

class WindowDrop : public context::Drop<WindowDrop>
{
public:
    WindowDrop(const std::string& name,
               const widget::layout::Rect& rect,
               context::StyleColorsDrop styleColorsDrop);

private:
    friend class Drop<WindowDrop>;
    static void pop();
    context::StyleColorsDrop styleColorsDrop;
};
} // namespace widget
