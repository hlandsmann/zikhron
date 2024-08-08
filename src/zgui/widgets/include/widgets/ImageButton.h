#pragma once
#include "detail/Widget.h" // IWYU pragma: export detail/Widget.h

#include <context/Texture.h> // IWYU pragma: export
#include <context/Theme.h>
#include <imgui.h>

#include <initializer_list>
#include <string>
#include <vector>

namespace widget {
using Images = std::initializer_list<context::Image>;

class ImageButton : public Widget
{
    template<class T>
    friend class MetaBox;
    friend class Widget;
    void setup(context::Image image);
    void setup(Images images);

public:
    ImageButton(WidgetInit init);
    ~ImageButton() override = default;

    ImageButton(const ImageButton&) = delete;
    ImageButton(ImageButton&&) = delete;
    auto operator=(const ImageButton&) -> ImageButton& = delete;
    auto operator=(ImageButton&&) -> ImageButton& = delete;

    auto clicked() -> bool;
    auto toggled(unsigned index) -> unsigned;

    template<class UnsignedEnum>
    auto toggled(UnsignedEnum enumVal) -> UnsignedEnum
    {
        return static_cast<UnsignedEnum>(toggled(static_cast<unsigned>(enumVal)));
    }

    void setChecked(bool checked);
    void setSensitive(bool sensitive);

    auto isChecked() const -> bool;
    auto isSensitive() const -> bool;

protected:
    auto calculateSize() const -> WidgetSize override;

private:
    ImVec4 backGroundColor;
    ImVec4 iconColor;
    bool sensitive{true};
    bool checked{false};
    std::string label;
    std::vector<context::Image> images;
};

}; // namespace widget
