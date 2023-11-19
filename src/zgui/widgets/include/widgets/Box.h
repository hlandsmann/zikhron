#pragma once
#include "Widget.h"

#include <imgui.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <utility>
#include <vector>

namespace widget {
class Box : public Widget<Box>
{
    using SizeType = layout::SizeType;
    using Align = layout::Align;

public:
    constexpr static float s_padding = 16.F;
    Box(layout::Orientation);
    Box(layout::Orientation, Align, std::shared_ptr<layout::Rect>);

    void arrange(const layout::Rect&);
    void arrange();

    template<class WidgetType, class... Args>
    auto add(Align widgetAlign, Args... args) -> std::shared_ptr<WidgetType>
    {
        auto widgetRect = std::make_shared<layout::Rect>();
        auto widgetOrientation = Orientation() == layout::Orientation::vertical
                                         ? layout::Orientation::horizontal
                                         : layout::Orientation::vertical;
        auto widget = std::make_shared<WidgetType>(widgetOrientation, widgetAlign, widgetRect, std::forward<Args>(args)...);
        widgets.push_back(static_cast<std::shared_ptr<WidgetBase>>(widget));
        // spdlog::warn("a: {}, w: {}, b: {}, f: {}",
        //              static_cast<int>(align),
        //              static_cast<int>(widget->Align()),
        //              static_cast<int>(widgets.back()->Align()),
        //              static_cast<int>(widgets.front()->Align()));
        rects.push_back(std::move(widgetRect));
        return widget;
    }
    template<class WidgetType>
    auto next() -> WidgetType&
    {
        auto& result = dynamic_cast<WidgetType&>(**currentWidgetIt);
        currentWidgetIt++;
        return result;
    }
    void start();

private:
    enum class Measure {
        width,
        height
    };
    Box(layout::Orientation, std::shared_ptr<layout::Rect>);
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
    std::vector<std::shared_ptr<WidgetBase>> widgets;
    std::vector<std::shared_ptr<layout::Rect>> rects;
    std::vector<std::shared_ptr<WidgetBase>>::iterator currentWidgetIt;

    std::shared_ptr<layout::Rect> layoutRect;
    float padding{s_padding};
};

} // namespace widget
