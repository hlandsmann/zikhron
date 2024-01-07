#pragma once
#include "detail/MetaBox.h"
#include "detail/Widget.h"

#include <memory>
#include <vector>
namespace widget {
class Layer : public MetaBox<Layer>
{
    friend class MetaBox<Layer>;
    using Align = layout::Align;
    using Orientation = layout::Orientation;
    using SizeType = layout::SizeType;

public:
    void setup();
    Layer(const WidgetInit& init);

    [[nodiscard]] auto arrange() -> bool override;

private:
    auto calculateSize() const -> WidgetSize override;
    layout::SizeType expandWidth{SizeType::width_expand};
    layout::SizeType expandHeight{SizeType::height_expand};
    std::vector<std::shared_ptr<Widget>> widgets;
    std::vector<std::shared_ptr<layout::Rect>> rects;
};

} // namespace widget
