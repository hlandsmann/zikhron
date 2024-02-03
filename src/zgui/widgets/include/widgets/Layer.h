#pragma once
#include "detail/MetaBox.h"
#include "detail/Widget.h"

#include <folly/sorted_vector_types.h>

#include <memory>
#include <vector>

namespace widget {
class Layer : public MetaBox<Layer>
{
    friend class MetaBox<Layer>;
    using Align = layout::Align;
    using ExpandType = layout::ExpandType;
    using Rect = layout::Rect;

public:
    void setup();
    Layer(const WidgetInit& init);

    [[nodiscard]] auto arrange(const layout::Rect& /* rect */) -> bool override;
    [[nodiscard]] auto getWidgetSizeFromRect(const layout::Rect& rect) -> WidgetSize override;
    void setAlignNewWidgetsVertical(Align newWidgetsVertical);

private:
    [[nodiscard]] auto calculateSize() const -> WidgetSize override;
    [[nodiscard]] auto calculateMinSize() const -> WidgetSize override;

    /* shared functions via MetaBox */
    auto newWidgetAlign(Align align, Measure measure) const -> Align;

    /* Layer internal functions */
    [[nodiscard]] static auto posNewWidget(Align align, float widgetSize, float rectSize) -> float;

    /* shared members via MetaBox */
    std::vector<std::shared_ptr<Widget>> widgets;
    std::vector<std::shared_ptr<layout::Rect>> rects;

    /* Layer internal Members */
    layout::ExpandType expandWidth{ExpandType::width_expand};
    layout::ExpandType expandHeight{ExpandType::height_expand};

    Align alignNewWidgetsVertical{Align::start};
};

} // namespace widget
