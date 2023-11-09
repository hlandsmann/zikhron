#pragma once
#include "Widget.h"

#include <imgui.h>

#include <memory>
#include <vector>

namespace widget {
class Layout : public Widget<Layout>
{
    using SizeType = layout::SizeType;
    using Align = layout::Align;

public:
    constexpr static float s_padding = 16.F;
    Layout(layout::Orientation);
    Layout(Align, std::shared_ptr<layout::Rect>, layout::Orientation);

    void setSize(const layout::Rect&);

    template<class WidgetType, class... Args>
    auto add(Align align, Args... args) -> std::shared_ptr<WidgetType>
    {
        auto rect = std::make_shared<layout::Rect>();
        auto widget = std::make_shared<WidgetType>(align, rect, std::forward<Args>(args)...);
        widgets.push_back(static_cast<std::shared_ptr<WidgetBase>>(widget));
        return widget;
    }

private:
    enum class Measure {
        width,
        height
    };
    Layout(std::shared_ptr<layout::Rect>, layout::Orientation _orientation);
    friend class Widget<Layout>;
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
    auto getWidgetNewCursor(Align align, float cursor, const std::shared_ptr<Widget>& widget,
                            float centerSize, float endSize, Measure measure) const -> float;
    static auto getWidgetNewSize(Align align, Align alignNextWidget,
                                 float cursor, float cursorNextWidget,
                                 const std::shared_ptr<Widget>& widget,
                                 Measure measure) -> float;
    void doLayout();

    layout::Orientation orientation;
    std::vector<std::shared_ptr<WidgetBase>> widgets;
    std::vector<std::shared_ptr<layout::Rect>> rects;

    std::shared_ptr<layout::Rect> rect;
    float padding{s_padding};
};

} // namespace widget
