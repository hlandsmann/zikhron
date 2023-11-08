#pragma once
#include "Widget.h"

#include <imgui.h>

#include <memory>
#include <vector>

namespace widget {
class Layout : public Widget<Layout>
{
  using SizeType = layout::SizeType;
public:
    constexpr static float s_padding = 16.F;
    Layout(layout::Orientation);
    Layout(layout::Align, std::shared_ptr<layout::Rect>, layout::Orientation);

    void setSize(const layout::Rect&);

    template<class WidgetType, class... Args>
    auto add(layout::Align align, Args... args) -> std::shared_ptr<WidgetType>
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
    static auto widgetSizeProjection(const std::shared_ptr<WidgetBase>& widget, Measure measure) -> float;
    static auto rectPositionProjection(const std::shared_ptr<layout::Rect>& rect, Measure measure) -> float&;
    static auto rectSizeProjection(const std::shared_ptr<layout::Rect>& rect, Measure measure) -> float&;
    auto accumulateMeasure(std::vector<std::shared_ptr<WidgetBase>>::const_iterator first,
                           std::vector<std::shared_ptr<WidgetBase>>::const_iterator last,
                           Measure measure) const -> float;
    static auto max_elementMeasure(std::vector<std::shared_ptr<WidgetBase>>::const_iterator first,
                                   std::vector<std::shared_ptr<WidgetBase>>::const_iterator last,
                                   Measure measure) -> float;
    static auto getNextAlign(layout::Align oldAlign, layout::Align nextAlign);
    void doLayout();

    layout::Orientation orientation;
    std::vector<std::shared_ptr<WidgetBase>> widgets;
    std::vector<std::shared_ptr<layout::Rect>> rects;

    std::shared_ptr<layout::Rect> rect;
    float padding{s_padding};
};

} // namespace widget
