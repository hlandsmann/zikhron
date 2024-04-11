#pragma once
#include <optional>
#include <string>
#include "detail/Widget.h" // IWYU pragma: export detail/Widget.h

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

    auto slide(float value) -> std::optional<float>;
private:
    auto calculateSize() const -> WidgetSize override;
    // auto calculateMinSize() const -> WidgetSize override;

    // auto arrange(const layout::Rect& rect) -> bool override;
    [[nodiscard]] auto getWidgetSizeFromRect(const layout::Rect& rect) -> WidgetSize override;

};

} // namespace widget
