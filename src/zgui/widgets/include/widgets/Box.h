#pragma once
#include "detail/MetaBox.h" // IWYU pragma: export detail/MetaBox.h
#include "detail/Widget.h"  // IWYU pragma: export detail/Widget.h

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
    using ExpandType = layout::ExpandType;

    template<class T>
    friend class MetaBox;
    friend class Widget;
    void setup(Orientation orientation);

public:
    Box(const WidgetInit& init);

    [[nodiscard]] auto arrange(const layout::Rect& rect) -> bool override;
    void setOrthogonalAlign(Align align);

private:
    [[nodiscard]] auto calculateSize(SizeType sizeType) const -> WidgetSize;
    [[nodiscard]] auto calculateSize() const -> WidgetSize override;
    [[nodiscard]] auto calculateMinSize() const -> WidgetSize override;

    /* shared functions via MetaBox */
    auto newWidgetAlign(Align align, Measure measure) const -> Align;

    /* Box internal functions */
    auto accumulateMeasure(std::vector<std::shared_ptr<Widget>>::const_iterator first,
                           std::vector<std::shared_ptr<Widget>>::const_iterator last,
                           Measure measure, SizeType sizeType) const -> float;
    static auto getNextAlign(Align oldAlign, Align nextAlign);
    void setChildWidgetsInitialRect();

    [[nodiscard]] static auto widgetNewRect(Measure measure,
                                            const layout::Rect& rect,
                                            float pos,
                                            float size,
                                            float orthogonalSize,
                                            const std::shared_ptr<Widget>& widget) -> layout::Rect;
    [[nodiscard]] static auto rectWithAdaptedSize(Measure measure, const layout::Rect& rect, float size) -> layout::Rect;
    [[nodiscard]] static auto oppositeMeasure(Measure measure) -> Measure;
    [[nodiscard]] static auto getSizeOfWidgetSize(Measure measure, WidgetSize widgetSize) -> float;
    [[nodiscard]] static auto getWidgetAlign(Measure measure, const std::shared_ptr<Widget>& widget) -> Align;
    [[nodiscard]] static auto getWidgetCursor(Measure measure,
                                              Align oldAlign,
                                              Align nextAlign,
                                              float centerSize, float endSize,
                                              const layout::Rect& rect,
                                              float oldCursor) -> float;
    [[nodiscard]] auto arrange(Measure measure, const layout::Rect& rect) -> bool;

    /* shared members via MetaBox */
    std::vector<std::shared_ptr<Widget>> widgets;

    /* Box internal Members */
    Orientation orientation{};
    layout::ExpandType expandWidth{ExpandType::width_expand};
    layout::ExpandType expandHeight{ExpandType::height_expand};
    layout::Align orthogonalAlign{Align::start};

    bool flipChildrensOrientation{true};
};

} // namespace widget
