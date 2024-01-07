#pragma once
#include "detail/MetaBox.h"
#include "detail/Widget.h"

#include <context/Theme.h>
#include <imgui.h>
#include <spdlog/spdlog.h>

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace widget {
class Box : public MetaBox<Box>
{
    friend class MetaBox<Box>;
    using Align = layout::Align;
    using Orientation = layout::Orientation;
    using SizeType = layout::SizeType;

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
    [[nodiscard]] auto calculateSize() const -> WidgetSize override;
    [[nodiscard]] auto getChildOrientation() const -> Orientation;
    auto newWidgetAlign(Align align, Measure measure) const -> Align;
    static auto widgetSizeTypeProjection(const WidgetSize& widgetSize, Measure measure) -> SizeType;
    static auto rectPositionProjection(layout::Rect& rect, Measure measure) -> float&;
    static auto rectSizeProjection(layout::Rect& rect, Measure measure) -> float&;
    auto accumulateMeasure(std::vector<std::shared_ptr<Widget>>::const_iterator first,
                           std::vector<std::shared_ptr<Widget>>::const_iterator last,
                           Measure measure) const -> float;
    static auto getNextAlign(Align oldAlign, Align nextAlign);
    auto getWidgetNewCursor(Align align, float cursor, const Widget& widget,
                            float centerSize, float endSize, Measure measure) const -> float;
    auto getWidgetNewSize(Align align, Align alignNextWidget,
                          float cursor, float cursorNextWidget,
                          const Widget& widget,
                          Measure measure) const -> float;
    void setChildWidgetsInitialRect();
    void doLayout(Measure measure);

    layout::Orientation orientation;
    layout::SizeType expandWidth{SizeType::width_expand};
    layout::SizeType expandHeight{SizeType::height_expand};
    layout::Align orthogonalAlign{Align::start};
    std::vector<std::shared_ptr<Widget>> widgets;
    std::vector<std::shared_ptr<layout::Rect>> rects;

    bool flipChildrensOrientation{true};
};

} // namespace widget
