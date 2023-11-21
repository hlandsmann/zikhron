#pragma once
#include "Widget.h"

#include <context/Texture.h>
#include <context/Theme.h>
#include <imgui.h>

#include <memory>

namespace widget {
enum class ib {
    cards,
};

class ImageButton : public Widget<ImageButton>
{
public:
    ImageButton(std::shared_ptr<context::Theme> theme,
                layout::Orientation orientation,
                layout::Align align,
                std::shared_ptr<layout::Rect> rect,
                context::Image image);
    ~ImageButton() override = default;

    ImageButton(const ImageButton&) = default;
    ImageButton(ImageButton&&) = default;
    auto operator=(const ImageButton&) -> ImageButton& = default;
    auto operator=(ImageButton&&) -> ImageButton& = default;

    auto clicked() const -> bool;

private:
    friend class Widget<ImageButton>;
    auto calculateSize() const -> WidgetSize;

    context::Image image;
};

}; // namespace widget
