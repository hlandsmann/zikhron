#pragma once
#include "detail/Widget.h"

#include <context/Texture.h>
#include <context/Theme.h>
#include <imgui.h>

#include <string>

namespace widget {

class ImageButton : public Widget
{
public:
    void setup(context::Image image);
    ImageButton(WidgetInit init);
    ~ImageButton() override = default;

    ImageButton(const ImageButton&) = default;
    ImageButton(ImageButton&&) = default;
    auto operator=(const ImageButton&) -> ImageButton& = default;
    auto operator=(ImageButton&&) -> ImageButton& = default;

    auto clicked() -> bool;
    void setChecked(bool checked);
    void setSensitive(bool sensitive);

protected:
    auto calculateSize() const -> WidgetSize override;

private:
    ImVec4 backGroundColor{};
    ImVec4 iconColor{};
    bool disabled{false};
    bool enabled{false};
    std::string label;
    context::Image image{};
};

}; // namespace widget
