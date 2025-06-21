#pragma once
#include "detail/Widget.h" // IWYU pragma: export detail/Widget.h

#include <context/Texture.h> // IWYU pragma: export
#include <context/Theme.h>
#include <imgui.h>

#include <filesystem>
#include <initializer_list>

namespace widget {
using Images = std::initializer_list<context::Image>;

class Image : public Widget
{
    template<class T>
    friend class MetaBox;
    friend class Widget;
    void setup(const std::filesystem::path& imageFile);

public:
    Image(WidgetInit init);
    ~Image() override = default;

    Image(const Image&) = delete;
    Image(Image&&) = delete;
    auto operator=(const Image&) -> Image& = delete;
    auto operator=(Image&&) -> Image& = delete;

    void draw() const;

protected:
    auto calculateSize() const -> WidgetSize override;

private:
    context::TextureData texture{};
};

}; // namespace widget
