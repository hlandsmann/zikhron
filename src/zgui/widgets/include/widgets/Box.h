#pragma once
#include "detail/MetaBox.h"
#include "detail/Widget.h"

#include <context/Theme.h>
#include <imgui.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <vector>

namespace widget {
class Box : public MetaBox<Box>
{
    friend class MetaBox<Box>;
    using Align = layout::Align;
    using Orientation = layout::Orientation;
    using ExpandType = layout::ExpandType;

public:
    void setup(){};
    Box(const WidgetInit& init);

    [[nodiscard]] auto arrange() -> bool override;
    void setOrientationHorizontal();
    void setOrientationVertical();
    void setFlipChildrensOrientation(bool flip);
    void setOrthogonalAlign(Align align);
    auto getExpandedSize() const -> WidgetSize;

private:
    [[nodiscard]] auto calculateSize(SizeType sizeType) const -> WidgetSize;
    [[nodiscard]] auto calculateSize() const -> WidgetSize override;
    [[nodiscard]] auto calculateMinSize() const -> WidgetSize override;

    /* shared functions via MetaBox */
    [[nodiscard]] auto getChildOrientation() const -> Orientation;
    auto newWidgetAlign(Align align, Measure measure) const -> Align;

    /* Box internal functions */
    static auto widgetExpandTypeProjection(const WidgetSize& widgetSize, Measure measure) -> ExpandType;
    static auto rectPositionProjection(layout::Rect& rect, Measure measure) -> float&;
    static auto rectSizeProjection(layout::Rect& rect, Measure measure) -> float&;
    auto accumulateMeasure(std::vector<std::shared_ptr<Widget>>::const_iterator first,
                           std::vector<std::shared_ptr<Widget>>::const_iterator last,
                           Measure measure, SizeType sizeType) const -> float;
    static auto getNextAlign(Align oldAlign, Align nextAlign);
    auto getWidgetNewCursor(Align align, float cursor, const Widget& widget,
                            float centerSize, float endSize, Measure measure) const -> float;
    auto getWidgetNewSize(Align align, Align alignNextWidget,
                          float cursor, float cursorNextWidget,
                          const Widget& widget,
                          Measure measure) const -> float;
    void setChildWidgetsInitialRect();
    void doLayout(Measure measure);

    /* shared members via MetaBox */
    std::vector<std::shared_ptr<Widget>> widgets;
    std::vector<std::shared_ptr<layout::Rect>> rects;

    /* Box internal Members */
    layout::Orientation orientation;
    layout::ExpandType expandWidth{ExpandType::width_expand};
    layout::ExpandType expandHeight{ExpandType::height_expand};
    layout::Align orthogonalAlign{Align::start};

    bool flipChildrensOrientation{true};
};

} // namespace widget
