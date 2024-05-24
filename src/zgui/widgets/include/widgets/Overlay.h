#pragma once
#include "Layer.h"
#include "TextToken.h"
#include "detail/Widget.h" // IWYU pragma: export detail/Widget.h

#include <context/Drop.h>
#include <context/Theme.h>

#include <functional>
#include <memory>
#include <string>

namespace widget {
class OverlayDrop;

class Overlay : public Widget
{
    using Align = layout::Align;
    template<class T>
    friend class MetaBox;
    friend class Widget;
    void setup(float maxWidth);

public:
    Overlay(WidgetInit init);
    ~Overlay() override = default;

    Overlay(const Overlay&) = default;
    Overlay(Overlay&&) = default;
    auto operator=(const Overlay&) -> Overlay& = default;
    auto operator=(Overlay&&) -> Overlay& = default;

    [[nodiscard]] auto dropOverlay(float x, float y) -> OverlayDrop;
    void setFirstDrop();
    [[nodiscard]] auto shouldClose() const -> bool;

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

private:
    [[nodiscard]] auto calculateSize() const -> WidgetSize override;
    auto arrange(const layout::Rect& rect) -> bool override;
    [[nodiscard]] auto getWidgetSizeFromRect(const layout::Rect& rect) -> WidgetSize override;

    [[nodiscard]] auto getLayerRect(const layout::Rect& rect) const -> layout::Rect;
    [[nodiscard]] auto getParentWindowRect() const -> layout::Rect;

    std::shared_ptr<widget::Layer> layer;
    float maxWidth{};

    bool firstDrop = false;
    bool closeNext = false;
    int framesActive = 0;
};

class OverlayDrop : public context::Drop<OverlayDrop>
{
public:
    OverlayDrop(const std::string& name,
                const widget::layout::Rect& rect,
                context::StyleColorsDrop styleColorsDrop,
                std::function<void()> execAfterWindowOpen);

private:
    friend class Drop<OverlayDrop>;
    static void pop();
    context::StyleColorsDrop styleColorsDrop;
};
} // namespace widget
