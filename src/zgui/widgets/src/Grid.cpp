#include <Grid.h>
#include <detail/MetaBox.h>
#include <detail/Widget.h>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <numeric>
#include <ranges>
#include <span>
#include <stdexcept>
#include <vector>
namespace ranges = std::ranges;
namespace views = std::ranges::views;

namespace widget {

void Grid::setup(std::size_t _columns, std::initializer_list<float> _priorities)
{
    columns = _columns;
    if (_priorities.size() > columns) {
        throw std::invalid_argument("to many priorities");
    }
    if (ranges::any_of(_priorities, [](float priority) { return priority > 1.F || priority < 0.F; })) {
        throw std::invalid_argument("bad priority");
    }
    ranges::copy(_priorities, std::back_inserter(priorities));
    auto sum = std::accumulate(priorities.begin(), priorities.end(), 0.F);
    ranges::transform(priorities, priorities.begin(), [sum](float priority) { return priority / sum; });
}

Grid::Grid(const WidgetInit& init)
    : MetaBox<Grid>{init}
{}

void Grid::mergeCell()
{
    std::size_t mergedCell = widgets.size() + mergedCells.size();
    if (!mergedCells.empty() && mergedCells.back() >= mergedCell) {
        throw std::runtime_error("merged Cells need to be sorted & unique");
    }
    if ((mergedCell + 1) % columns == 0) {
        throw std::runtime_error("can't merge last cell of row");
    }
    mergedCells.push_back(mergedCell);
}

auto Grid::arrange(const layout::Rect& rect) -> bool
{
    bool needArrange = false;

    auto cursorsX = std::vector<float>(columns + 1, rect.x);
    auto cursorsY = std::vector<float>(1, rect.y);
    // auto sizes = std::vector<float>{};

    traverseWidgets(cursorsX, cursorsY,
                    rect,
                    ExpandType::width_fixed,
                    [](const std::shared_ptr<Widget>& widget, float /* cursorX */, float /* cursorY */, float /* width */)
                            -> WidgetSize {
                        return widget->getWidgetMinSize();
                    });
    traverseWidgets(cursorsX, cursorsY,
                    rect,
                    ExpandType::width_fixed,
                    [&rect](const std::shared_ptr<Widget>& widget, float /* cursorX */, float /* cursorY */, float width)
                            -> WidgetSize {
                        return widget->getWidgetSizeFromRect({.x = 0, .y = 0, .width = width, .height = rect.height});
                    });
    traverseWidgets(cursorsX, cursorsY,
                    rect,
                    ExpandType::width_expand,
                    [&rect](const std::shared_ptr<Widget>& widget, float /* cursorX */, float /* cursorY */, float width)
                            -> WidgetSize {
                        return widget->getWidgetSizeFromRect({.x = 0, .y = 0, .width = width, .height = rect.height});
                    });
    traverseWidgets(cursorsX, cursorsY,
                    rect,
                    ExpandType::width_fixed,
                    [&rect, &needArrange](const std::shared_ptr<Widget>& widget, float cursorX, float cursorY, float width)
                            -> WidgetSize {
                        needArrange |= widget->arrange({.x = cursorX, .y = cursorY, .width = width, .height = rect.height});
                        return widget->getWidgetSize();
                    });
    width = cursorsX.back() - cursorsX.front();
    height = cursorsY.back() - cursorsY.front();

    return needArrange;
}

void Grid::traverseWidgets(std::vector<float>& cursorsX,
                           std::vector<float>& cursorsY,
                           const layout::Rect& rect,
                           ExpandType expandPriority,
                           std::function<WidgetSize(const std::shared_ptr<Widget>& widget,
                                                    float cursorX,
                                                    float cursorY,
                                                    float width)>
                                   fun) const
{
    std::size_t column = 0;
    std::size_t cellCounter = 0;
    auto itMergedCells = mergedCells.cbegin();
    std::size_t cursorIndexStart = 0;
    std::size_t cursorIndexEnd = 0;
    for (const auto& widget : widgets) {
        column = nextColumn(itMergedCells, cellCounter);
        std::size_t row = (cellCounter - 1) / columns;
        cursorIndexEnd = (column == 0) ? columns : column + 1;
        float cursorX = cursorsX[cursorIndexStart];
        float& cursorY = getVectorIndexElement(cursorsY, row);

        float availableWidth = getAvailableWidth(cursorIndexStart, cursorIndexEnd, cursorsX, rect.width, expandPriority);
        auto widgetSize = fun(widget, cursorX, cursorY, availableWidth);
        setCursor(cursorsX, cursorIndexEnd, cursorX + widgetSize.width);
        setCursor(cursorsY, row + 1, cursorY + widgetSize.height);

        cursorIndexStart = cursorIndexEnd == columns ? 0 : cursorIndexEnd;
    }
}

auto Grid::nextColumn(std::vector<std::size_t>::const_iterator& itMergedCell, std::size_t& cellCounter) const -> std::size_t
{
    std::size_t column = cellCounter % columns;
    while (itMergedCell != mergedCells.end() && column == *itMergedCell) {
        cellCounter++;
        itMergedCell++;
    }
    cellCounter++;

    column = cellCounter % columns;
    return column;
}

auto Grid::getAvailableWidth(std::size_t indexStart,
                             std::size_t indexEnd,
                             const std::vector<float>& cursors,
                             float fullWidth,
                             ExpandType expandType) const -> float
{
    auto accumulate = [](auto range) { float sum=0.F; for(auto val:range){ sum+=val;} return sum; };
    auto cellCursors = std::span(cursors.begin() + static_cast<int>(indexStart), cursors.begin() + static_cast<int>(indexEnd));
    auto cellPriorities = std::span(priorities.begin() + static_cast<int>(indexStart), priorities.begin() + static_cast<int>(indexEnd));

    auto adj_diff = [](float a, float b) { return b - a; };
    auto columnWidths = cursors | views::adjacent_transform<2>(adj_diff);
    auto cellWidths = cellCursors | views::adjacent_transform<2>(adj_diff);
    auto cellPriority = accumulate(cellPriorities);

    float usedCellWidth = accumulate(cellWidths);
    float usedGridWidth = accumulate(columnWidths);
    float residueWidth = fullWidth - usedGridWidth - usedCellWidth;
    return expandType == ExpandType::width_expand
                   ? residueWidth
                   : std::min(residueWidth, cellPriority * fullWidth);
}

void Grid::setCursor(std::vector<float>& cursors, std::size_t index, float value)
{
    if (index >= cursors.size()) {
        float initVal = cursors.empty() ? 0.F : cursors.back();
        cursors.resize(index + 1, initVal);
    }
    float oldValue = cursors[index];
    float difference = value - oldValue;
    difference = std::max(0.F, difference);
    auto span = std::span(std::next(cursors.begin(), static_cast<int>(index)), cursors.end());
    ranges::transform(span, span.begin(), [=](float val) { return val + difference; });
}

template<class T>
auto Grid::getVectorIndexElement(std::vector<T>& vector, std::size_t index) -> T&
{
    if (index >= vector.size()) {
        const auto currentSize = vector.size();
        vector.resize(index + 1);
        std::fill(std::next(vector.begin(), static_cast<int>(currentSize)), vector.end(), T{});
    }
    return vector[index];
}

auto Grid::getWidgetSizeFromRect(const layout::Rect& rect) -> WidgetSize
{
    return {};
}

auto Grid::calculateSize() const -> WidgetSize
{
    return {.width = width,
            .height = height};
}

auto Grid::calculateMinSize() const -> WidgetSize
{
    auto cursorsX = std::vector<float>(columns + 1);
    auto cursorsY = std::vector<float>(1);
    // auto sizes = std::vector<float>{};

    traverseWidgets(cursorsX, cursorsY,
                    layout::Rect{},
                    ExpandType::width_fixed,
                    [](const std::shared_ptr<Widget>& widget, float /* cursorX */, float /* cursorY */, float /* width */)
                            -> WidgetSize {
                        return widget->getWidgetMinSize();
                    });
    return {.width = cursorsX.back(),
            .height = cursorsY.back()};
}

} // namespace widget
