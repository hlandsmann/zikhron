#pragma once
#include "Widget.h"

#include <fmt/core.h>
#include <fmt/format.h> // IWYU pragma: export core.h
#include <folly/sorted_vector_types.h>

#include <cstddef>
#include <magic_enum.hpp>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace widget {

enum class Orientation {
    horizontal,
    vertical
};
class Layer;
class Grid;
class Box;

template<class BoxImpl>
class MetaBox : public Widget
{
    using Align = layout::Align;
    using ExpandType = layout::ExpandType;

public:
    MetaBox(const WidgetInit& init);

    void setPadding(float padding);
    void setBorder(float border);

    template<class WidgetType, class... Args>
    auto add(Align widgetAlign, Args... args) -> std::shared_ptr<WidgetType>
    {
        auto* self = static_cast<BoxImpl*>(this);
        auto widgetRect = std::make_shared<layout::Rect>();
        WidgetInit init = {.theme = getThemePtr(),
                           .widgetIdGenerator = getWidgetIdGenerator(),
                           .rect = widgetRect,
                           .horizontalAlign = self->newWidgetAlign(widgetAlign, Measure::horizontal),
                           .verticalAlign = self->newWidgetAlign(widgetAlign, Measure::vertical),
                           .parent = shared_from_this()};
        if constexpr (std::is_same_v<BoxImpl, Layer>
                      && (std::is_same_v<WidgetType, Box> || std::is_same_v<WidgetType, Grid>)) {
            init.expandTypeWidth = getExpandTypeWidth();
            init.expandTypeHeight = getExpandTypeHeight();
        }
        auto widget = std::make_shared<WidgetType>(std::move(init));
        auto newWidgetId = widget->getWidgetId();
        widget->setup(std::forward<Args>(args)...);
        if constexpr (std::is_same_v<WidgetType, BoxImpl>) {
            widget->setPadding(padding);
        }
        self->widgets.push_back(static_cast<std::shared_ptr<Widget>>(widget));
        id_widgets[newWidgetId] = widget;

        resetWidgetSize();
        setArrangeIsNecessary();
        return widget;
    }

    void pop();
    void clear();

    template<class WidgetType>
    auto getWidget(WidgetId _widgetId) -> WidgetType&
    {
        return dynamic_cast<WidgetType&>(*id_widgets.at(_widgetId));
    }

    template<class WidgetType>
    auto next() -> WidgetType&
    {
        auto* self = static_cast<BoxImpl*>(this);
        if (!(currentWidgetIt < self->widgets.end())) {
            throw std::out_of_range("Bad call of next(). Forgot to call start()?");
        }
        auto& result = dynamic_cast<WidgetType&>(**currentWidgetIt);
        currentWidgetIt++;
        return result;
    }

    [[nodiscard]] auto isLast() const -> bool;
    void start();
    auto numberOfWidgets() const -> std::size_t;

    constexpr static float s_padding = 16.F;

protected:
    auto calculateMinSize() const -> WidgetSize override = 0;
    auto getBorderedRect(const layout::Rect& rect) const -> layout::Rect;
    auto getBorder() const -> float;
    auto getPadding() const -> float;

    enum class SizeType {
        min,
        standard,
    };
    enum class Measure {
        horizontal,
        vertical
    };
    static auto getWidgetAlign(const Widget& widget, Measure measure) -> Align;
    static void setWidgetAlign(Widget& widget, Measure measure, Align align);
    static auto widgetSizeProjection(Measure measure, const WidgetSize& widgetSize) -> float;
    static auto rectPositionProjection(Measure measure, const layout::Rect& rect) -> float;
    static auto rectSizeProjection(Measure measure, const layout::Rect& rect) -> float;
    static auto max_elementMeasure(std::vector<std::shared_ptr<Widget>>::const_iterator first,
                                   std::vector<std::shared_ptr<Widget>>::const_iterator last,
                                   Measure measure, SizeType) -> float;

private:
    float padding{s_padding};
    float border{};
    std::vector<std::shared_ptr<Widget>>::iterator currentWidgetIt;
    folly::sorted_vector_map<WidgetId, std::shared_ptr<Widget>> id_widgets;
};

} // namespace widget

template<>
struct fmt::formatter<widget::Orientation>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(widget::Orientation orientation, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(), "{}", magic_enum::enum_name(orientation));
    }
};
