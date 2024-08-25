#pragma once
#include "detail/Widget.h" // IWYU pragma: export detail/Widget.h

#include <cstddef>
#include <string>

namespace widget {

class SteppedSlider : public Widget
{
    using Align = layout::Align;
    using ExpandType = layout::ExpandType;

    template<class T>
    friend class MetaBox;
    friend class Widget;
    void setup();

public:
    SteppedSlider(const WidgetInit& init);
    ~SteppedSlider() override = default;
    SteppedSlider(const SteppedSlider&) = delete;
    SteppedSlider(SteppedSlider&&) = delete;
    auto operator=(const SteppedSlider&) -> SteppedSlider& = delete;
    auto operator=(SteppedSlider&&) -> SteppedSlider& = delete;

    auto slide(std::size_t max, std::size_t pos) -> std::size_t;

private:
    static auto labelString(std::size_t max, std::size_t pos) -> std::string;
    auto calculateSize() const -> WidgetSize override;
    [[nodiscard]] auto getWidgetSizeFromRect(const layout::Rect& rect) -> WidgetSize override;

    unsigned lastValue{};
};

} // namespace widget
