#pragma once
#include "detail/Widget.h" // IWYU pragma: export detail/Widget.h

#include <string>

namespace widget {

class MediaSlider : public Widget
{
    using Align = layout::Align;
    using ExpandType = layout::ExpandType;

    template<class T>
    friend class MetaBox;
    friend class Widget;
    void setup();

public:
    MediaSlider(const WidgetInit& init);

    auto slide(double start, double end, double pos) -> double;

private:
    static auto sliderValueFromPos(double start, double end, double pos) -> float;
    static auto posFromSliderValue(double start, double end, float value) -> double;
    static auto timeString(double start, double pos) -> std::string;
    auto calculateSize() const -> WidgetSize override;
    [[nodiscard]] auto getWidgetSizeFromRect(const layout::Rect& rect) -> WidgetSize override;

    float lastValue{};
};

} // namespace widget
