#include <Fonts.h>
#include <Texture.h>
#include <Theme.h>
#include <imgui.h>

#include <utility>
namespace context {

Theme::Theme(Fonts _fonts, Texture _texture)
    : fonts{std::move(_fonts)}
    , texture{std::move(_texture)}
{}

auto Theme::ColorButton() const -> const ImVec4&
{
    return colorButton;
}

auto Theme::ColorButtonHovered() const -> const ImVec4&
{
    return colorButtonHovered;
}

auto Theme::ColorButtonActive() const -> const ImVec4&
{
    return colorButtonActive;
}

auto Theme::ColorToggleButtonEnabled() const -> const ImVec4&
{
    return colorToggleButtonEnabled;
}

auto Theme::ColorToggleButtonEnabledHovered() const -> const ImVec4&
{
    return colorToggleButtonEnabledHovered;
}

auto Theme::ColorToggleButtonDisabled() const -> const ImVec4&
{
    return colorToggleButtonDisabled;
}

auto Theme::ColorToggleButtonDisabledHovered() const -> const ImVec4&
{
    return colorToggleButtonDisabledHovered;
}

auto Theme::ColorImage() const -> const ImVec4&
{
    return colorImage;
}

auto Theme::ColorImageHovered() const -> const ImVec4&
{
    return colorImageHovered;
}

auto Theme::ColorImageActive() const -> const ImVec4&
{
    return colorImageActive;
}

auto Theme::ColorImageEnabled() const -> const ImVec4&
{
    return colorImageEnabled;
}

auto Theme::ColorImageEnabledHovered() const -> const ImVec4&
{
    return colorImageEnabledHovered;
}

auto Theme::ColorImageDisabled() const -> const ImVec4&
{
    return colorImageDisabled;
}

auto Theme::ColorImageDisabledHovered() const -> const ImVec4&
{
    return colorImageDisabledHovered;
}

auto Theme::getFont() const -> const Fonts&
{
    return fonts;
}

auto Theme::getTexture() const -> const Texture&
{
    return texture;
}
} // namespace context
