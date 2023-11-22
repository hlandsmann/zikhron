#pragma once
#include "Widget.h"

#include <context/Texture.h>
#include <context/Theme.h>
#include <imgui.h>

#include <memory>
#include <string>

namespace widget {
enum class ib {
    cards,
};

class ImageButton : public Widget<ImageButton>
{
public:
    ImageButton(WidgetInit init,
                std::string label,
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

    ImVec4 backGroundColor{};
    ImVec4 iconColor{};

    std::string label;
    context::Image image;
};

}; // namespace widget
