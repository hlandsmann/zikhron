#pragma once
#include "detail/MetaBox.h" // IWYU pragma: export detail/MetaBox.h
#include "detail/Widget.h"  // IWYU pragma: export detail/Widget.h

#include <cstddef>
#include <functional>
#include <memory>
#include <vector>

namespace widget {
class Grid : public MetaBox<Grid>
{
public:
    using Priorities = std::vector<float>;

private:
    friend class MetaBox<Grid>;
    using Align = layout::Align;
    using ExpandType = layout::ExpandType;

    template<class T>
    friend class MetaBox;
    friend class Widget;
    void setup(std::size_t columns, Priorities priorities);
    void setup(const BoxCfg& boxCfg, std::size_t columns, Priorities priorities);

public:
    Grid(const WidgetInit& init);
    ~Grid() override = default;

    Grid(const Grid&) = delete;
    Grid(Grid&&) = delete;
    auto operator=(const Grid&) -> Grid& = delete;
    auto operator=(Grid&&) -> Grid& = delete;

    // Merges current active cell with next one. Throws if current active cell is last in row.
    // Use only when filling the grid with widgets.
    void mergeCell();
    void autoSetColumnsPaddingRearrange(float minPadding, float maxPadding);

private:
    [[nodiscard]] auto arrange(const layout::Rect& rect) -> bool override;
    void traverseWidgets(const layout::Rect& rect,
                         std::vector<float>& sizesX,
                         std::vector<float>& sizesY,
                         std::vector<WidgetSize>& widgetSizes,
                         ExpandType expandPriority,
                         std::function<WidgetSize(
                                 const std::shared_ptr<Widget>& widget,
                                 float cursorX,
                                 float cursor,
                                 const WidgetSize& widgetSize,
                                 float availableWidth,
                                 float width,
                                 float height)>
                                 fun) const;
    void calculateWidgetSizes(const layout::Rect& rect,
                              std::vector<float>& sizesX,
                              std::vector<float>& sizesY,
                              std::vector<WidgetSize>& widgetSizes) const;
    auto getAvailableWidth(const std::vector<float>& sizesX,
                           std::size_t column,
                           float fullWidth,
                           ExpandType expandType) const -> float;
    auto widgetSizeFromSizes(const std::vector<float>& sizesX, const std::vector<float>& sizesY) const -> WidgetSize;
    [[nodiscard]] static auto alignShiftPos(Align align, float pos, float size, float availableSize) -> float;

    /* functions overriden from Widget */
    [[nodiscard]] auto getWidgetSizeFromRect(const layout::Rect& rect) -> WidgetSize override;
    [[nodiscard]] auto calculateSize() const -> WidgetSize override;
    [[nodiscard]] auto calculateMinSize() const -> WidgetSize override;

    /* shared functions via MetaBox */
    static auto newWidgetAlign(Align align, Measure measure) -> Align;

    /* shared members via MetaBox */
    std::vector<std::shared_ptr<Widget>> widgets;

    /* Grid internal Members */
    std::size_t columns{};
    std::vector<std::size_t> mergedCells;
    std::vector<float> priorities;
    WidgetSize arrangedSize{};
    WidgetSize gridWidgetSize{};
};

} // namespace widget
