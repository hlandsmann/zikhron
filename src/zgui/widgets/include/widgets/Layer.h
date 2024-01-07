#pragma once
#include "detail/MetaBox.h"
#include "detail/Widget.h"

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <vector>
namespace widget {
class Layer : public MetaBox<Layer>
{
    friend class MetaBox<Layer>;
    using Align = layout::Align;
    using Orientation = layout::Orientation;
    using SizeType = layout::SizeType;
    using Rect = layout::Rect;

public:
    void setup();
    Layer(const WidgetInit& init);

    [[nodiscard]] auto arrange() -> bool override;
    void setAlignNewWidgetsVertical(Align newWidgetsVertical);

    template<class WidgetType>
    auto getLayer(std::size_t index) -> WidgetType&
    {
        return dynamic_cast<WidgetType&>(*widgets.at(index));
    }

private:
    auto calculateSize() const -> WidgetSize override;

    /* shared functions via MetaBox */
    [[nodiscard]] auto getChildOrientation() const -> Orientation;
    auto newWidgetAlign(Align align, Measure measure) const -> Align;

    /* Layer internal functions */
    void doLayout();
    static auto sizeNewWidget(const Widget& widget, const Rect& borderedRect, Measure measure) -> float;
    static auto posNewWidget(const Widget& widget, const Rect& borderedRect, Measure measure) -> float;

    /* shared members via MetaBox */
    std::vector<std::shared_ptr<Widget>> widgets;
    std::vector<std::shared_ptr<layout::Rect>> rects;

    /* Layer internal Members */
    layout::SizeType expandWidth{SizeType::width_expand};
    layout::SizeType expandHeight{SizeType::height_expand};

    Align alignNewWidgetsVertical{Align::start};
};

} // namespace widget
