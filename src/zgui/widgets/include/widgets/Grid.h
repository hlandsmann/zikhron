#pragma once
#include "detail/MetaBox.h" // IWYU pragma: export core.h
#include "detail/Widget.h"

#include <cstddef>
#include <functional>
#include <initializer_list>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

namespace widget {
class Grid : public MetaBox<Grid>
{
    friend class MetaBox<Grid>;
    using Align = layout::Align;
    using ExpandType = layout::ExpandType;

public:
    void setup(std::size_t columns, std::initializer_list<float> priorities);
    Grid(const WidgetInit& init);

    // Merges current active cell with next one. Throws if current active cell is last in row.
    // Use only when filling the grid with widgets.
    void mergeCell();

private:
    [[nodiscard]] auto arrange(const layout::Rect& /* rect */) -> bool override;
    void traverseWidgets(std::vector<float>& cursorsX,
                         std::vector<float>& cursorsY,
                         const layout::Rect& rect,
                         ExpandType expandPriority,
                         std::function<WidgetSize(
                                 const std::shared_ptr<Widget>& widget,
                                 float cursorX,
                                 float cursorY,
                                 float width)>
                                 fun) const;
    [[nodiscard]] auto nextColumn(std::vector<std::size_t>::const_iterator& itMergedCell, std::size_t& cellCounter) const -> std::size_t;
    [[nodiscard]] auto getAvailableWidth(std::size_t indexStart,
                                         std::size_t indexEnd,
                                         const std::vector<float>& cursors,
                                         float fullWidth,
                                         ExpandType expandType) const -> float;
    static void setCursor(std::vector<float>& cursors, std::size_t index, float value);
    template<class T>
    [[nodiscard]] static auto getVectorIndexElement(std::vector<T>& vector, std::size_t index) -> T&;

    /* functions overriden from Widget */
    [[nodiscard]] auto getWidgetSizeFromRect(const layout::Rect& rect) -> WidgetSize override;
    [[nodiscard]] auto calculateSize() const -> WidgetSize override;
    [[nodiscard]] auto calculateMinSize() const -> WidgetSize override;

    /* shared functions via MetaBox */
    [[nodiscard]] auto getChildOrientation() const -> Orientation;
    auto newWidgetAlign(Align align, Measure measure) const -> Align;

    /* Grid internal functions */
    auto getRowHeight(std::size_t row) const -> float;
    auto getColumnWidth(std::size_t column) const -> float;

    /* shared members via MetaBox */
    std::vector<std::shared_ptr<Widget>> widgets;

    /* Grid internal Members */
    std::size_t columns{};
    std::vector<std::size_t> mergedCells;
    std::vector<float> priorities;
    float gridWidth{};
    float gridHeight{};
};

} // namespace widget
