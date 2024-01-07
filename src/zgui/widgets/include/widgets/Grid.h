#pragma once
#include "detail/MetaBox.h"
#include "detail/Widget.h"

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>
namespace widget {
class Grid : public MetaBox<Grid>
{
    friend class MetaBox<Grid>;
    using Align = layout::Align;
    using Orientation = layout::Orientation;
    using SizeType = layout::SizeType;

public:
    void setup(std::size_t rows);
    Grid(const WidgetInit& init);

    [[nodiscard]] auto arrange() -> bool override;

private:
    auto calculateSize() const -> WidgetSize override;
    auto getRowHeight(std::size_t row) const -> float;
    auto getColumnWidth(std::size_t column) const -> float;
    std::size_t rows{};
    std::vector<std::shared_ptr<Widget>> widgets;
    std::vector<std::shared_ptr<layout::Rect>> rects;
};

} // namespace widget
