#pragma once
#include "Widget.h"

#include <context/Theme.h>
#include <imgui.h>
#include <spdlog/spdlog.h>

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

namespace widget {
class Box : public Widget<Box>
{
    using SizeType = layout::SizeType;
    using Align = layout::Align;

public:
    constexpr static float s_padding = 16.F;
    Box(std::shared_ptr<context::Theme> theme, layout::Orientation);
    Box(const WidgetInit& init);

    void arrange(const layout::Rect&);
    void arrange();
    void setBorder(float border);
    void setOrientationHorizontal();
    void setOrientationVertical();
    void setFlipChildrensOrientation(bool flip);
    void setPadding(float padding);

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
                           .align = widgetAlign};
        auto widget = std::make_shared<WidgetType>(std::move(init), std::forward<Args>(args)...);
        widgets.push_back(static_cast<std::shared_ptr<WidgetBase>>(widget));
        rects.push_back(std::move(widgetRect));
        resetWidgetSize();
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

private:
    enum class Measure {
        width,
        height
    };
    Box(std::shared_ptr<context::Theme> theme, layout::Orientation, std::shared_ptr<layout::Rect>);
    friend class Widget<Box>;
    auto calculateSize() const -> WidgetSize;
    static auto widgetSizeProjection(const WidgetSize& widgetSize, Measure measure) -> float;
    static auto widgetSizeTypeProjection(const WidgetSize& widgetSize, Measure measure) -> SizeType;
    static auto rectPositionProjection(layout::Rect& rect, Measure measure) -> float&;
    static auto rectSizeProjection(layout::Rect& rect, Measure measure) -> float&;
    auto accumulateMeasure(std::vector<std::shared_ptr<WidgetBase>>::const_iterator first,
                           std::vector<std::shared_ptr<WidgetBase>>::const_iterator last,
                           Measure measure) const -> float;
    static auto max_elementMeasure(std::vector<std::shared_ptr<WidgetBase>>::const_iterator first,
                                   std::vector<std::shared_ptr<WidgetBase>>::const_iterator last,
                                   Measure measure) -> float;
    static auto getNextAlign(Align oldAlign, Align nextAlign);
    auto getWidgetNewCursor(Align align, float cursor, const WidgetBase& widget,
                            float centerSize, float endSize, Measure measure) const -> float;
    auto getWidgetNewSize(Align align, Align alignNextWidget,
                          float cursor, float cursorNextWidget,
                          const WidgetBase& widget,
                          Measure measure) const -> float;
    void setChildWidgetsInitialRect();
    void doLayout(Measure measure);

    layout::Orientation orientation;
    float border{};
    std::vector<std::shared_ptr<WidgetBase>> widgets;
    std::vector<std::shared_ptr<layout::Rect>> rects;
    std::vector<std::shared_ptr<WidgetBase>>::iterator currentWidgetIt;

    std::shared_ptr<layout::Rect> layoutRect;
    std::shared_ptr<layout::Rect> borderedRect;
    float padding{s_padding};
    bool flipChildrensOrientation{true};
};

} // namespace widget
