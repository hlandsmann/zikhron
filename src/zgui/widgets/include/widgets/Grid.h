#pragma once
#include "detail/Widget.h"

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>
namespace widget {
class Grid : public Widget
{
    using Align = layout::Align;
    using Orientation = layout::Orientation;
    using SizeType = layout::SizeType;

public:
    void setup(std::size_t rows);
    Grid(const WidgetInit& init);

    [[nodiscard]] auto arrange() -> bool override;
    void setPadding(float padding);
    void setBorder(float border);

    template<class WidgetType, class... Args>
    auto add(Align widgetAlign, Args... args) -> std::shared_ptr<WidgetType>
    {
        auto widgetRect = std::make_shared<layout::Rect>();
        WidgetInit init = {.theme = getThemePtr(),
                           .widgetIdGenerator = getWidgetIdGenerator(),
                           .rect = widgetRect,
                           .orientation = Orientation::horizontal,
                           .align = widgetAlign,
                           .parent = shared_from_this()};
        auto widget = std::make_shared<WidgetType>(std::move(init));
        widget->setup(std::forward<Args>(args)...);
        widgets.push_back(static_cast<std::shared_ptr<Widget>>(widget));
        rects.push_back(std::move(widgetRect));
        resetWidgetSize();
        setArrangeIsNecessary();
        return widget;
    }
    void pop();
    void clear();

    void start();
    template<class WidgetType>
    auto next() -> WidgetType&
    {
        if (!(currentWidgetIt < widgets.end())) {
            throw std::out_of_range("Bad call of next(). Forgot to call start()?");
        }
        auto& result = dynamic_cast<WidgetType&>(**currentWidgetIt);
        currentWidgetIt++;
        return result;
    }
    [[nodiscard]] auto isLast() const -> bool;

    constexpr static float s_padding = 16.F;

private:
    auto calculateSize() const -> WidgetSize override;
    auto getRowHeight(std::size_t row) const -> float;
    auto getColumnWidth(std::size_t column) const -> float;
    std::size_t rows{};
    float padding{s_padding};
    float border{};
    std::shared_ptr<layout::Rect> borderedRect;
    std::vector<std::shared_ptr<Widget>> widgets;
    std::vector<std::shared_ptr<layout::Rect>> rects;
    std::vector<std::shared_ptr<Widget>>::iterator currentWidgetIt;
};

} // namespace widget
