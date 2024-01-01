#pragma once
#include "Widget.h"

#include <context/Theme.h>
#include <imgui.h>
#include <spdlog/spdlog.h>

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace widget {
class Box : public Widget
{
    using SizeType = layout::SizeType;
    using Align = layout::Align;
    void setup(){};

public:
    constexpr static float s_padding = 16.F;
    Box(std::shared_ptr<context::Theme> theme, layout::Orientation, std::weak_ptr<Widget> parent);
    Box(const WidgetInit& init);

    // [[nodiscard]] auto arrange(const layout::Rect&) -> bool;
    [[nodiscard]] auto arrange() -> bool override;
    void setBorder(float border);
    void setOrientationHorizontal();
    void setOrientationVertical();
    void setFlipChildrensOrientation(bool flip);
    void setPadding(float padding);
    auto getExpandedSize() const -> WidgetSize;

    template<class WidgetType, class... Args>
    auto add(Align widgetAlign, Args... args) -> std::shared_ptr<WidgetType>
    {
        auto widgetRect = std::make_shared<layout::Rect>();
        auto widgetOrientation = PassiveOrientation() == layout::Orientation::vertical && flipChildrensOrientation
                                         ? layout::Orientation::horizontal
                                         : layout::Orientation::vertical;
        WidgetInit init = {.theme = getThemePtr(),
                           .rect = widgetRect,
                           .orientation = widgetOrientation,
                           .align = widgetAlign,
                           .parent = shared_from_this()};
        auto widget = std::make_shared<WidgetType>(std::move(init));
        widget->setup(std::forward<Args>(args)...);
        if constexpr (std::is_same_v<WidgetType, Box>) {
            widget->setPadding(padding);
        }

        widgets.push_back(static_cast<std::shared_ptr<Widget>>(widget));
        rects.push_back(std::move(widgetRect));
        resetWidgetSize();
        setArrangeIsNecessary();
        return widget;
    }

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
    void clear();
    void pop();
    void start();
    auto numberOfWidgets() const -> std::size_t;

protected:
    auto calculateSize() const -> WidgetSize override;

private:
    enum class Measure {
        width,
        height
    };
    Box(std::shared_ptr<context::Theme> theme, layout::Orientation, std::shared_ptr<layout::Rect>, std::weak_ptr<Widget> parent);
    static auto widgetSizeProjection(const WidgetSize& widgetSize, Measure measure) -> float;
    static auto widgetSizeTypeProjection(const WidgetSize& widgetSize, Measure measure) -> SizeType;
    static auto rectPositionProjection(layout::Rect& rect, Measure measure) -> float&;
    static auto rectSizeProjection(layout::Rect& rect, Measure measure) -> float&;
    auto accumulateMeasure(std::vector<std::shared_ptr<Widget>>::const_iterator first,
                           std::vector<std::shared_ptr<Widget>>::const_iterator last,
                           Measure measure) const -> float;
    static auto max_elementMeasure(std::vector<std::shared_ptr<Widget>>::const_iterator first,
                                   std::vector<std::shared_ptr<Widget>>::const_iterator last,
                                   Measure measure) -> float;
    static auto getNextAlign(Align oldAlign, Align nextAlign);
    auto getWidgetNewCursor(Align align, float cursor, const Widget& widget,
                            float centerSize, float endSize, Measure measure) const -> float;
    auto getWidgetNewSize(Align align, Align alignNextWidget,
                          float cursor, float cursorNextWidget,
                          const Widget& widget,
                          Measure measure) const -> float;
    void setChildWidgetsInitialRect();
    void doLayout(Measure measure);

    layout::Orientation orientation;
    layout::SizeType expandWidth{SizeType::width_expand};
    layout::SizeType expandHeight{SizeType::height_expand};
    float border{};
    std::vector<std::shared_ptr<Widget>> widgets;
    std::vector<std::shared_ptr<layout::Rect>> rects;
    std::vector<std::shared_ptr<Widget>>::iterator currentWidgetIt;

    std::shared_ptr<layout::Rect> borderedRect;
    float padding{s_padding};
    bool flipChildrensOrientation{true};
};

} // namespace widget
