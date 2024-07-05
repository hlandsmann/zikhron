#pragma once
#include "Layer.h"
#include "detail/Widget.h" // IWYU pragma: export detail/Widget.h

#include <context/Drop.h>
#include <context/Theme.h>

#include <memory>
#include <string>

namespace widget {

class ChildDrop;

class Child : public Widget
{
    using Align = layout::Align;
    using ExpandType = layout::ExpandType;

    template<class T>
    friend class MetaBox;
    friend class Widget;
    friend class Overlay;
    void setup(float width, const std::string& name);
    void setup(const std::string& name);

public:
    Child(const WidgetInit& init);
    ~Child() override = default;

    Child(const Child&) = default;
    Child(Child&&) = default;
    auto operator=(const Child&) -> Child& = default;
    auto operator=(Child&&) -> Child& = default;

    auto arrange(const layout::Rect& rect) -> bool override;

    [[nodiscard]] auto dropChild() -> ChildDrop;

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
    auto calculateSize() const -> WidgetSize override;
    auto calculateMinSize() const -> WidgetSize override;

private:
    float width{};
    std::shared_ptr<widget::Layer> layer;
};

class ChildDrop : public context::Drop<ChildDrop>
{
public:
    ChildDrop(const std::string& name,
              const widget::layout::Rect& rect,
              context::StyleColorsDrop styleColorsDrop);

private:
    friend class Drop<ChildDrop>;
    static void pop();
    context::StyleColorsDrop styleColorsDrop;
};
} // namespace widget
