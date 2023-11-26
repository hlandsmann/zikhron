#pragma once
#include "Widget.h"

#include <context/Texture.h>
#include <context/Theme.h>
#include <imgui.h>

#include <string>

namespace widget {

class ImageButton : public Widget<ImageButton>
{
public:
    ImageButton(WidgetInit init,
                context::Image image);
    ~ImageButton() override = default;

    ImageButton(const ImageButton&) = default;
    ImageButton(ImageButton&&) = default;
    auto operator=(const ImageButton&) -> ImageButton& = default;
    auto operator=(ImageButton&&) -> ImageButton& = default;

    auto clicked() -> bool;
    void setChecked(bool checked);
    void setSensitive(bool sensitive);

private:
    friend class Widget<ImageButton>;
    auto calculateSize() const -> WidgetSize;

    ImVec4 backGroundColor{};
    ImVec4 iconColor{};
    bool disabled{false};
    bool enabled{false};
    std::string label;
    context::Image image;
};

}; // namespace widget
