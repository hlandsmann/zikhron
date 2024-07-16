#include <Grid.h>
#include <context/imglog.h>
#include <detail/MetaBox.h>
#include <detail/Widget.h>
#include <utils/format.h>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <numeric>
#include <ranges>
#include <stdexcept>
#include <utility>
#include <vector>
namespace ranges = std::ranges;
namespace views = std::ranges::views;

namespace widget {

void Grid::setup(std::size_t _columns, Priorities _priorities)
{
    columns = _columns;
    if (_priorities.size() > columns) {
        throw std::invalid_argument("to many priorities");
    }
    if (ranges::any_of(_priorities, [](float priority) { return priority > 1.F || priority < 0.F; })) {
        throw std::invalid_argument("bad priority");
    }
    priorities.clear();
    ranges::copy(_priorities, std::back_inserter(priorities));
    auto sum = std::accumulate(priorities.begin(), priorities.end(), 0.F);
    ranges::transform(priorities, priorities.begin(), [sum](float priority) { return priority / sum; });
}

void Grid::setup(const BoxCfg& boxCfg, std::size_t _columns, Priorities _priorities)
{
    setCfg(boxCfg);
    setup(_columns, _priorities);
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

void Grid::autoSetColumnsPaddingRearrange(float minPadding, float maxPadding)
{
    float width = getBorder() * 2;
    float padding = 0.F;

    std::size_t newColumns = 0;
    for (const auto& widget : widgets) {
        width += padding + widget->getWidgetSize().width;
        padding = minPadding;
        if (width <= arrangedSize.width) {
            newColumns++;
        } else {
            width -= padding + widget->getWidgetSize().width;
            break;
        }
    }
    newColumns = std::max(newColumns, std::size_t{1});
    if (newColumns != 1) {
        width -= padding * static_cast<float>(newColumns - 1);
        float availableWidth = arrangedSize.width - width;
        padding = availableWidth / static_cast<float>(newColumns - 1);
    }
    padding = std::clamp(padding, minPadding, maxPadding);
    setHorizontalPadding(padding);
    auto newPriorities = Priorities(newColumns, 1.F / static_cast<float>(newColumns));

    setup(newColumns, newPriorities);
}

auto Grid::arrange(const layout::Rect& rect) -> bool
{
    bool needArrange = false;
    if (widgets.empty()) {
        gridWidgetSize = {};
        return needArrange;
    }
    arrangedSize = {.width = rect.width, .height = rect.height};

    auto sizesX = std::vector<float>(columns, 0.F);
    auto sizesY = std::vector<float>{};
    auto widgetSizes = std::vector<WidgetSize>{widgets.size()};

    calculateWidgetSizes(rect, sizesX, sizesY, widgetSizes);

    traverseWidgets(rect,
                    sizesX, sizesY, widgetSizes,
                    ExpandType::width_expand,
                    [&](const std::shared_ptr<Widget>& widget,
                        float cursorX, float cursorY,
                        const WidgetSize& widgetSize,
                        float /* availableWidth */,
                        float width, float height)
                            -> WidgetSize {
                        needArrange |= widget->arrange({.x = cursorX,
                                                        .y = alignShiftPos(widget->VerticalAlign(), cursorY, widgetSize.height, height),
                                                        .width = width,
                                                        .height = height});
                        auto widgetS = widget->getWidgetSize();
                        return widgetS;
                    });

    gridWidgetSize = widgetSizeFromSizes(sizesX, sizesY);
    return needArrange;
}

void Grid::traverseWidgets(const layout::Rect& rect,
                           std::vector<float>& sizesX,
                           std::vector<float>& sizesY,
                           std::vector<WidgetSize>& widgetSizes,
                           ExpandType expandPriority,
                           std::function<WidgetSize(
                                   const std::shared_ptr<Widget>& widget,
                                   float cursorX, float cursorY,
                                   const WidgetSize& widgetSize,
                                   float availableWidth,
                                   float width, float height)>
                                   fun) const
{
    const auto& borderedRect = getBorderedRect(rect);
    float cursorX = borderedRect.x;
    float cursorY = borderedRect.y;
    float rowMaxSizeY = 0.F;
    for (const auto& [index, widget] : views::enumerate(widgets)) {
        auto column = static_cast<std::size_t>(index) % columns;
        auto row = static_cast<std::size_t>(index) / columns;
        if (sizesY.size() <= row) {
            sizesY.resize(row + 1);
        }

        float& sizeX = sizesX.at(column);
        float& sizeY = sizesY.at(row);
        WidgetSize& widgetSize = widgetSizes.at(static_cast<std::size_t>(index));
        float availableWidth = getAvailableWidth(sizesX, column, borderedRect.width, expandPriority);
        widgetSize = fun(widget, cursorX, cursorY, widgetSize, availableWidth, sizeX, sizeY);
        sizeX = std::max(widgetSize.width, sizeX);
        rowMaxSizeY = std::max(widgetSize.height, rowMaxSizeY);

        cursorX += sizeX + getHorizontalPadding();
        if ((static_cast<std::size_t>(index) + 1) % columns == 0
            || static_cast<std::size_t>(index + 1) == numberOfWidgets()) {
            cursorX = borderedRect.x;
            cursorY += rowMaxSizeY;
            cursorY += getVerticalPadding();
            sizeY = rowMaxSizeY;
            rowMaxSizeY = 0.F;
        }
    }
}

void Grid::calculateWidgetSizes(const layout::Rect& rect,
                                std::vector<float>& sizesX,
                                std::vector<float>& sizesY,
                                std::vector<WidgetSize>& widgetSizes) const
{
    traverseWidgets(rect,
                    sizesX, sizesY, widgetSizes,
                    ExpandType::width_fixed,
                    [](const std::shared_ptr<Widget>& widget,
                       float /* cursorX */, float /* cursorY */,
                       const WidgetSize& /* widgetSize */,
                       float /* availableWidth */,
                       float /* width */, float /* height */)
                            -> WidgetSize {
                        auto minSize = widget->getWidgetMinSize();
                        return minSize;
                    });
    traverseWidgets(rect,
                    sizesX, sizesY, widgetSizes,
                    ExpandType::width_fixed,
                    [&rect](const std::shared_ptr<Widget>& widget,
                            float /* cursorX */, float /* cursorY */,
                            const WidgetSize& /* widgetSize */,
                            float availableWidth,
                            float /* width */, float /* height */)
                            -> WidgetSize {
                        auto widgetS = widget->getWidgetSizeFromRect({.x = 0, .y = 0, .width = availableWidth, .height = rect.height});
                        return widgetS;
                    });

    traverseWidgets(rect,
                    sizesX, sizesY, widgetSizes,
                    ExpandType::width_expand,
                    [&rect](const std::shared_ptr<Widget>& widget,
                            float /* cursorX */, float /* cursorY */,
                            const WidgetSize& /* widgetSize */,
                            float availableWidth,
                            float /* width */, float /* height */)
                            -> WidgetSize {
                        auto widgetS = widget->getWidgetSizeFromRect({.x = 0, .y = 0, .width = availableWidth, .height = rect.height});
                        return widgetS;
                    });
}

auto Grid::getAvailableWidth(const std::vector<float>& sizesX,
                             std::size_t column,
                             float fullWidth,
                             ExpandType expandType) const -> float
{
    if (sizesX.empty()) {
        return 0.F;
    }
    fullWidth -= getHorizontalPadding() * static_cast<float>(sizesX.size() - 1);

    float usedWidth = std::accumulate(sizesX.begin(), sizesX.end(), 0.F);
    usedWidth -= sizesX.at(column);
    float availableWidth = std::max(0.F, fullWidth - usedWidth);
    return expandType == ExpandType::expand
                   ? availableWidth
                   : std::min(priorities.at(column) * fullWidth, availableWidth);
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
    if (widgets.empty()) {
        return {};
    }
    auto sizesX = std::vector<float>(columns, 0.F);
    auto sizesY = std::vector<float>{};
    auto widgetSizes = std::vector<WidgetSize>{widgets.size()};
    calculateWidgetSizes(rect, sizesX, sizesY, widgetSizes);
    auto widgetSize = widgetSizeFromSizes(sizesX, sizesY);
    return widgetSize;
}

auto Grid::widgetSizeFromSizes(const std::vector<float>& sizesX, const std::vector<float>& sizesY) const -> WidgetSize
{
    return {
            .width = std::accumulate(sizesX.begin(), sizesX.end(),
                                     getBorder() * 2 + getHorizontalPadding() * std::max(0.F, static_cast<float>(sizesX.size() - 1))),
            .height = std::accumulate(sizesY.begin(), sizesY.end(),
                                      getBorder() * 2 + getVerticalPadding() * std::max(0.F, static_cast<float>(sizesY.size() - 1))),
    };
}

auto Grid::calculateSize() const -> WidgetSize
{
    return gridWidgetSize;
}

auto Grid::calculateMinSize() const -> WidgetSize
{
    auto sizesX = std::vector<float>(columns, 0.F);
    auto sizesY = std::vector<float>{};
    auto widgetSizes = std::vector<WidgetSize>{widgets.size()};
    auto rect = layout::Rect{};
    traverseWidgets(rect,
                    sizesX, sizesY, widgetSizes,
                    ExpandType::width_fixed,
                    [](const std::shared_ptr<Widget>& widget,
                       float /* cursorX */, float /* cursorY */,
                       const WidgetSize& /* widgetSize */,
                       float /* availableWidth */,
                       float /* width */, float /* height */)
                            -> WidgetSize {
                        auto minSize = widget->getWidgetMinSize();
                        return minSize;
                    });
    return widgetSizeFromSizes(sizesX, sizesY);
}

auto Grid::newWidgetAlign(Align align, Measure /* measure */) -> Align
{
    return align;
}
} // namespace widget
