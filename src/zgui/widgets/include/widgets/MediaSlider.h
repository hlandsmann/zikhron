#pragma once
#include <string>
#include "detail/Widget.h" // IWYU pragma: export detail/Widget.h

namespace widget {

class MediaSlider : public Widget
{
    using Align = layout::Align;
    using ExpandType = layout::ExpandType;
public:
    void setup(float minHeight,
               ExpandType expandTypeHeight);
    MediaSlider(const WidgetInit& init);
    ~MediaSlider() override = default;
};

} // namespace widget
