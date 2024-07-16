#pragma once
#include "detail/MetaBox.h" // IWYU pragma: export detail/MetaBox.h
#include "detail/Widget.h"  // IWYU pragma: export detail/Widget.h

#include <context/Theme.h>
#include <imgui.h>
#include <spdlog/spdlog.h>

#include <functional>
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
    void setup(const BoxCfg& boxCfg, Orientation orientation);

public:
    Box(const WidgetInit& init);
    ~Box()override = default;
    Box(const Box&) = delete;
    Box(Box&&) = delete;
    auto operator=(const Box&) -> Box& = delete;
    auto operator=(Box&&) -> Box& = delete;

    [[nodiscard]] auto arrange(const layout::Rect& rect) -> bool override;
    [[nodiscard]] auto getWidgetSizeFromRect(const layout::Rect& rect) -> WidgetSize override;
    void setOrthogonalAlign(Align align);

private:
    [[nodiscard]] auto calculateSize() const -> WidgetSize override;
    [[nodiscard]] auto calculateMinSize() const -> WidgetSize override;

    /* shared functions via MetaBox */
    auto newWidgetAlign(Align align, Measure measure) const -> Align;

    /* Box internal functions */
    auto accumulateMeasure(std::vector<std::shared_ptr<Widget>>::const_iterator first,
                           std::vector<std::shared_ptr<Widget>>::const_iterator last,
                           Measure measure, SizeType sizeType) const -> float;
    auto accumulateMeasure(std::vector<WidgetSize>::const_iterator first,
                           std::vector<WidgetSize>::const_iterator last,
                           Measure measure) const -> float;

    static auto getNextAlign(Align oldAlign, Align nextAlign);

    [[nodiscard]] static auto oppositeMeasure(Measure measure) -> Measure;
    [[nodiscard]] static auto getWidgetAlign(Measure measure, const std::shared_ptr<Widget>& widget) -> Align;
    [[nodiscard]] static auto getWidgetCursor(Measure measure,
                                              Align oldAlign,
                                              Align nextAlign,
                                              float centerSize, float endSize,
                                              const layout::Rect& rect,
                                              float oldCursor) -> float;
    void calculateWidgetSizes(const layout::Rect& rect,
                              std::vector<WidgetSize>& widgetSizes) const;
    [[nodiscard]] auto arrange(Measure measure, const layout::Rect& rect) -> bool;
    [[nodiscard]] static auto getAvailableSize(float fullSize,
                                               float startSize, float centerSize, float endSize,
                                               Align align) -> float;

    void traverseWidgets(const layout::Rect& rect,
                         std::vector<WidgetSize>& widgetSizes,
                         std::function<WidgetSize(
                                 const std::shared_ptr<Widget>& widget,
                                 float cursorX, float cursorY,
                                 float width, float height,
                                 float availableWidth, float availableHeight,
                                 const WidgetSize& widgetSize)>
                                 fun) const;

    /* shared members via MetaBox */
    std::vector<std::shared_ptr<Widget>> widgets;

    /* Box internal Members */
    Orientation orientation{};
    layout::Align orthogonalAlign{Align::start};

    WidgetSize boxWidgetSize;
};

} // namespace widget
