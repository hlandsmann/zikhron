#pragma once
#include "detail/Widget.h" // IWYU pragma: export detail/Widget.h

#include <context/Texture.h> // IWYU pragma: export
#include <context/Theme.h>
#include <imgui.h>

#include <string>

namespace widget {

class ImageButton : public Widget
{
    template<class T>
    friend class MetaBox;
    friend class Widget;
    void setup(context::Image image);
    void setup(context::Image imageActionOpen, context::Image imageActionClose);

public:
    ImageButton(WidgetInit init);
    ~ImageButton() override = default;

    ImageButton(const ImageButton&) = delete;
    ImageButton(ImageButton&&) = delete;
    auto operator=(const ImageButton&) -> ImageButton& = delete;
    auto operator=(ImageButton&&) -> ImageButton& = delete;

    auto clicked() -> bool;
    auto isOpen() -> bool;
    void setOpen(bool open);
    void setChecked(bool checked);
    void setSensitive(bool sensitive);
    

    auto isChecked() const ->bool;
    auto isSensitive() const -> bool;
protected:
    auto calculateSize() const -> WidgetSize override;

private:
    ImVec4 backGroundColor;
    ImVec4 iconColor;
    bool sensitive{true};
    bool checked{false};
    std::string label;
    bool open{false};
    context::Image image{};
    context::Image imageActionOpen{};
    context::Image imageActionClose{};

};

}; // namespace widget
