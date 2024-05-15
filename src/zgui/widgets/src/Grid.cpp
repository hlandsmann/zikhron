// #include <spdlog/fmt/bundled/core.h>
// #include <spdlog/fmt/bundled/format.h>
#include <Grid.h>
#include <context/imglog.h>
#include <detail/MetaBox.h>
#include <detail/Widget.h>
#include <utils/spdlog.h>

#include <algorithm>
#include <cstddef>
#include <deque>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <numeric>
#include <ranges>
#include <span>
#include <stdexcept>
#include <utility>
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

    // imglog::log("{}:, x: {}, y: {}, w: {}, h: {} --------------------------", getName(), rect.x, rect.y, rect.width, rect.height);
    traverseWidgets(cursorsX, cursorsY,
                    rect,
                    ExpandType::width_fixed,
                    [](const std::shared_ptr<Widget>& widget,
                       float /* cursorX */, float /* cursorY */,
                       float /* width */, float /* height */)
                            -> WidgetSize {
                        auto minSize = widget->getWidgetMinSize();
                        // if (widget->getName() == "ttq_1") {
                        //     imglog::log("in0 {}:,  wsw: {}, wsh: {}", widget->getName(), minSize.width, minSize.height);
                        // }
                        return minSize;
                    });
    // std::string logstr = fmt::format("{}", fmt::join(cursorsX, ", "));
    // imglog::log("cursors: {} --------------------------", logstr);
    traverseWidgets(cursorsX, cursorsY,
                    rect,
                    ExpandType::width_fixed,
                    [&rect](const std::shared_ptr<Widget>& widget,
                            float /* cursorX */, float /* cursorY */,
                            float width, float /* height */)
                            -> WidgetSize {
                        auto widgetS = widget->getWidgetSizeFromRect({.x = 0, .y = 0, .width = width, .height = rect.height});
                        // imglog::log("in:,  w: {}, h: {}", width, rect.height);
                        // if (widget->getName() == "ttq_1") {
                        //     imglog::log("in1 {}:,  wsw: {}, wsh: {}, w: {}, h: {}", widget->getName(), widgetS.width, widgetS.height, width, rect.height);
                        // }
                        return widgetS;
                    });
    // logstr = fmt::format("{}", fmt::join(cursorsX, ", "));
    // imglog::log("cursors: {} --------------------------", logstr);
    ranges::fill(cursorsY, rect.y);
    std::deque<float> heights;
    traverseWidgets(cursorsX, cursorsY,
                    rect,
                    ExpandType::width_expand,
                    [&rect, &heights](const std::shared_ptr<Widget>& widget,
                                      float /* cursorX */, float /* cursorY */,
                                      float width, float /* height */)
                            -> WidgetSize {
                        // imglog::log("in2:,  w: {}, h: {}", width, rect.height);
                        auto widgetS = widget->getWidgetSizeFromRect({.x = 0, .y = 0, .width = width, .height = rect.height});
                        heights.push_back(widgetS.height);
                        // if (widget->getName() == "ttq_1") {
                        //     imglog::log("in2:, {}:  wsw: {}, wsh: {}, w: {}, h: {}", widget->getName(), widgetS.width, widgetS.height, width, rect.height);
                        // }
                        return widgetS;
                    });
    // logstr = fmt::format("{}", fmt::join(cursorsX, ", "));
    // imglog::log("cursors: {} --------------------------", logstr);
    traverseWidgets(cursorsX, cursorsY,
                    rect,
                    ExpandType::width_fixed,
                    [&rect, &heights, &needArrange](const std::shared_ptr<Widget>& widget,
                                          float cursorX, float cursorY,
                                          float width, float height)
                            -> WidgetSize {
                            float widgetHeight = heights.front();
                            heights.pop_front();
                        needArrange |= widget->arrange({.x = cursorX, 
                            .y = alignShiftPos(widget->VerticalAlign(), cursorY, widgetHeight, height),
                            .width = width, 
                            .height = rect.height});
                        auto widgetS = widget->getWidgetSize();
                        // if (widget->getName() == "ttq_1") {
                        //     imglog::log("arrange_grid, {}: wsw: {}, wsh: {}, w: {}, h: {}, cx: {}, cy: {}", widget->getName(), widgetS.width, widgetS.height, width, rect.height, cursorX, cursorY);
                        // }
                        return widgetS;
                    });
    // auto logstr = fmt::format("{}", fmt::join(cursorsY, ", "));
    // imglog::log("n: {}: cursors: {}", getName(), logstr);
    gridWidth = cursorsX.back() - cursorsX.front();
    gridHeight = cursorsY.back() - cursorsY.front();
    // std::string logstr = fmt::format("{}", fmt::join(cursorsY, ", "));
    // imglog::log("cursors: {} --------------------------", logstr);

    return needArrange;
}

void Grid::traverseWidgets(std::vector<float>& cursorsX,
                           std::vector<float>& cursorsY,
                           const layout::Rect& rect,
                           ExpandType expandPriority,
                           std::function<WidgetSize(const std::shared_ptr<Widget>& widget,
                                                    float cursorX, float cursorY,
                                                    float width, float height)>
                                   fun) const
{
    std::size_t cellCounter = 0;
    auto itMergedCells = mergedCells.cbegin();
    std::size_t cursorIndexStart = 0;
    std::size_t cursorIndexEnd = 0;
    for (const auto& widget : widgets) {
        cursorIndexEnd = nextCursorIndexEnd(itMergedCells, cellCounter);
        std::size_t row = (cellCounter - 1) / columns;
        float cursorX = cursorsX[cursorIndexStart];
        float cursorY = getVectorIndexElement(cursorsY, row);

        float availableWidth = getAvailableWidth(cursorIndexStart, cursorIndexEnd, cursorsX, rect.width, expandPriority);
        float availableHeight = cursorsY.size() > row + 1 ? std::max(0.F, cursorsY[row + 1] - cursorY) : 0;
        auto widgetSize = fun(widget, cursorX, cursorY, availableWidth, availableHeight);
        // imglog::log("start: {}, end: {}, cellcounter: {}, widgetWidth: {}",
        // cursorIndexStart, cursorIndexEnd, cellCounter, widgetSize.width);
        setCursor(cursorsX, cursorIndexEnd, cursorX + widgetSize.width);
        setCursor(cursorsY, row + 1, cursorY + widgetSize.height);

        cursorIndexStart = cursorIndexEnd == columns ? 0 : cursorIndexEnd;
    }
}

auto Grid::nextCursorIndexEnd(std::vector<std::size_t>::const_iterator& itMergedCell, std::size_t& cellCounter) const -> std::size_t
{
    while (itMergedCell != mergedCells.end() && cellCounter == *itMergedCell) {
        cellCounter++;
        itMergedCell++;
    }
    std::size_t cursorIndexEnd = (cellCounter % columns) + 1;
    cellCounter++;

    return cursorIndexEnd;
}

auto Grid::getAvailableWidth(std::size_t indexStart,
                             std::size_t indexEnd,
                             const std::vector<float>& cursors,
                             float fullWidth,
                             ExpandType expandType) const -> float
{
    auto accumulate = [](auto range) { float sum=0.F; for(auto val:range){ sum+=val;} return sum; };

    // a cell can encompass multiple cursors (it can span over multiple columns)
    auto cellCursors = std::span(cursors.begin() + static_cast<int>(indexStart),
                                 cursors.begin() + static_cast<int>(indexEnd + 1));
    auto cellPriorities = std::span(priorities.begin() + static_cast<int>(indexStart),
                                    priorities.begin() + static_cast<int>(indexEnd));

    auto adj_diff = [](float a, float b) { return b - a; };
    auto columnWidths = cursors | views::adjacent_transform<2>(adj_diff);
    auto cellWidths = cellCursors | views::adjacent_transform<2>(adj_diff);
    auto cellPriority = accumulate(cellPriorities);

    float usedCellWidth = accumulate(cellWidths);
    float usedGridWidth = accumulate(columnWidths);
    float residueWidth = fullWidth - (usedGridWidth - usedCellWidth);
    // imglog::log("is: {}, ie: {}", indexStart, indexEnd);
    // imglog::log("fw: {}, ugw: {},  ucw: {}, rw: {}, cpfw: {}, cp: {}", fullWidth, usedGridWidth, usedCellWidth, residueWidth, cellPriority * fullWidth, cellPriority);
    // std::string cursorsStr = fmt::format("{}", fmt::join(cursors, ", "));
    // std::string cellWidthsStr = fmt::format("{}", fmt::join(cellWidths, ", "));
    // std::string columnWidthsStr = fmt::format("{}", fmt::join(columnWidths, ", "));
    // std::string cellPrioritiesStr = fmt::format("{}", fmt::join(cellPriorities, ", "));
    // imglog::log("cursors: {} ++ cellWidths: {} ++ columnWidths: {} ++ cellPriorities: {}, cellPriority: {}, residue: {}",
    //             cursorsStr, cellWidthsStr, columnWidthsStr, cellPrioritiesStr, cellPriority, residueWidth);
    return expandType == ExpandType::width_expand
                   ? residueWidth
                   : std::max(usedCellWidth, std::min(residueWidth, cellPriority * fullWidth));
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
        // const auto currentSize = vector.size();
        vector.resize(index + 1);
        // std::fill(std::next(vector.begin(), static_cast<int>(currentSize)), vector.end(), T{});
    }
    return vector[index];
}

auto Grid::alignShiftPos(Align align, float pos, float size, float availableSize) -> float
{
    switch (align) {
    case Align::start:
        return pos;
    case Align::center:
        return pos + ((availableSize - size) / 2.F);
    case Align::end:
        return pos + (availableSize - size);
    }
    std::unreachable();
}

auto Grid::getWidgetSizeFromRect(const layout::Rect& rect) -> WidgetSize
{
    return {.width = rect.width, .height = rect.height};
}

auto Grid::calculateSize() const -> WidgetSize
{
    return {.width = gridWidth,
            .height = gridHeight};
}

auto Grid::calculateMinSize() const -> WidgetSize
{
    auto cursorsX = std::vector<float>(columns + 1);
    auto cursorsY = std::vector<float>(1);
    // auto sizes = std::vector<float>{};

    traverseWidgets(cursorsX, cursorsY,
                    layout::Rect{},
                    ExpandType::width_fixed,
                    [](const std::shared_ptr<Widget>& widget,
                       float /* cursorX */, float /* cursorY */,
                       float /* width */, float /* height */)
                            -> WidgetSize {
                        return widget->getWidgetMinSize();
                    });
    return {.width = cursorsX.back(),
            .height = cursorsY.back()};
}

auto Grid::newWidgetAlign(Align align, Measure /* measure */) -> Align
{
    return align;
}
} // namespace widget
