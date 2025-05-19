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
    ~MediaSlider() override = default;
    MediaSlider(const MediaSlider&) = delete;
    MediaSlider(MediaSlider&&) = delete;
    auto operator=(const MediaSlider&) -> MediaSlider& = delete;
    auto operator=(MediaSlider&&) -> MediaSlider& = delete;

    auto slide(double start, double end, double pos) -> double;
    void setUseKeyboard(bool useKeyboard);

private:
    [[nodiscard]] static auto valueFromKeyboard(float value) -> float;
    [[nodiscard]] static auto sliderValueFromPos(double start, double end, double pos) -> float;
    [[nodiscard]] static auto posFromSliderValue(double start, double end, float value) -> double;
    [[nodiscard]] static auto timeString(double start, double pos) -> std::string;
    [[nodiscard]] auto calculateSize() const -> WidgetSize override;
    [[nodiscard]] auto getWidgetSizeFromRect(const layout::Rect& rect) -> WidgetSize override;

    float lastValue{};
    bool useKeyboard{false};
};

} // namespace widget
