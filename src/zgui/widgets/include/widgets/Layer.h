#pragma once
#include "detail/MetaBox.h" // IWYU pragma: export detail/MetaBox.h
#include "detail/Widget.h"  // IWYU pragma: export detail/Widget.h

#include <memory>
#include <vector>

namespace widget {
class Layer : public MetaBox<Layer>
{
    friend class MetaBox<Layer>;
    using Align = layout::Align;
    using ExpandType = layout::ExpandType;
    using Rect = layout::Rect;

    template<class T>
    friend class MetaBox;
    friend class Widget;
    void setup();

public:
    Layer(const WidgetInit& init);
    ~Layer()override = default;
    Layer(const Layer&) = delete;
    Layer(Layer&&) = delete;
    auto operator=(const Layer&) -> Layer& = delete;
    auto operator=(Layer&&) -> Layer& = delete;

    [[nodiscard]] auto arrange(const layout::Rect& /* rect */) -> bool override;
    [[nodiscard]] auto getWidgetSizeFromRect(const layout::Rect& rect) -> WidgetSize override;
    void setAlignNewWidgetsVertical(Align newWidgetsVertical);

    void setHidden(bool hidden);
    [[nodiscard]] auto isHidden() const -> bool;

private:
    [[nodiscard]] auto calculateSize() const -> WidgetSize override;
    [[nodiscard]] auto calculateMinSize() const -> WidgetSize override;

    /* shared functions via MetaBox */
    auto newWidgetAlign(Align align, Measure measure) const -> Align;

    /* Layer internal functions */
    [[nodiscard]] static auto posNewWidget(Align align, float pos, float widgetSize, float rectSize) -> float;

    /* shared members via MetaBox */
    std::vector<std::shared_ptr<Widget>> widgets;
    std::vector<std::shared_ptr<layout::Rect>> rects;

    /* Layer internal Members */
    layout::ExpandType expandWidth{ExpandType::width_expand};
    layout::ExpandType expandHeight{ExpandType::height_expand};

    Align alignNewWidgetsVertical{Align::start};

    bool hidden{false};
};

} // namespace widget
