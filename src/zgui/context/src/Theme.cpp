#include <Fonts.h>
#include <Texture.h>
#include <Theme.h>
#include <WidgetState.h>
#include <imgui.h>

#include <cstddef>
#include <utility>

namespace context {

Theme::Theme(Fonts _fonts, Texture _texture)
    : fonts{std::move(_fonts)}
    , texture{std::move(_texture)}
{}

auto Theme::dropImGuiStyleVars() -> StyleVarsDrop
{
    return {};
}

auto Theme::dropImGuiStyleColors(ColorTheme colorTheme) const -> StyleColorsDrop
{
    return {*this, colorTheme};
}

auto Theme::ColorButton(WidgetState state) const -> const ImVec4&
{
    switch (state) {
    case WidgetState::default_state:
        return ColorButton();
    case WidgetState::hovered:
        return ColorButtonHovered();
    case WidgetState::active:
        return ColorButtonActive();
    case WidgetState::checked:
        return ColorToggleButtonChecked();
    case WidgetState::checked_hovered:
        return ColorToggleButtonCheckedHovered();
    case WidgetState::insensitive:
        return ColorToggleButtonInsensitive();
    case WidgetState::insensitive_hovered:
        return ColorToggleButtonInsensitiveHovered();
    }
    std::unreachable();
}

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

auto Theme::ColorToggleButtonChecked() const -> const ImVec4&
{
    return colorToggleButtonChecked;
}

auto Theme::ColorToggleButtonCheckedHovered() const -> const ImVec4&
{
    return colorToggleButtonCheckedHovered;
}

auto Theme::ColorToggleButtonInsensitive() const -> const ImVec4&
{
    return colorToggleButtonInsensitive;
}

auto Theme::ColorToggleButtonInsensitiveHovered() const -> const ImVec4&
{
    return colorToggleButtonInsensitiveHovered;
}

auto Theme::ColorImage(WidgetState state) const -> const ImVec4&
{
    switch (state) {
    case WidgetState::default_state:
        return ColorImage();
    case WidgetState::hovered:
        return ColorImageHovered();
    case WidgetState::active:
        return ColorImageActive();
    case WidgetState::checked:
        return ColorImageChecked();
    case WidgetState::checked_hovered:
        return ColorImageCheckedHovered();
    case WidgetState::insensitive:
        return ColorImageInsensitive();
    case WidgetState::insensitive_hovered:
        return ColorImageInsensitiveHovered();
    }
    std::unreachable();
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

auto Theme::ColorImageChecked() const -> const ImVec4&
{
    return colorImageChecked;
}

auto Theme::ColorImageCheckedHovered() const -> const ImVec4&
{
    return colorImageCheckedHovered;
}

auto Theme::ColorImageInsensitive() const -> const ImVec4&
{
    return colorImageInsensitive;
}

auto Theme::ColorImageInsensitiveHovered() const -> const ImVec4&
{
    return colorImageInsensitiveHovered;
}

auto Theme::ColorWindowBackground() const -> const ImVec4&
{
    return colorWindowBackground;
}

auto Theme::ColorOverlayBackground() const -> const ImVec4&
{
    return colorOverlayBackground;
}

auto Theme::getFont() const -> const Fonts&
{
    return fonts;
}

auto Theme::getTexture() const -> const Texture&
{
    return texture;
}

StyleVarsDrop::StyleVarsDrop()
{
    PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0F, 0.0F));
    PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0F, 0.0F));
}

void StyleVarsDrop::PushStyleVar(ImGuiStyleVar idx, float val)
{
    ImGui::PushStyleVar(idx, val);
    incPopCount();
}

void StyleVarsDrop::PushStyleVar(ImGuiStyleVar idx, const ImVec2& val)
{
    ImGui::PushStyleVar(idx, val);
    incPopCount();
}

void StyleVarsDrop::pop()
{
    ImGui::PopStyleVar();
}

StyleColorsDrop::StyleColorsDrop(const Theme& theme, ColorTheme colorTheme)
{
    switch (colorTheme) {
    case ColorTheme::ButtonDefault:
        PushStyleColor(ImGuiCol_Button, theme.ColorButton());
        PushStyleColor(ImGuiCol_ButtonHovered, theme.ColorButtonHovered());
        PushStyleColor(ImGuiCol_ButtonActive, theme.ColorButtonActive());
        break;
    case ColorTheme::ButtonChecked:
        PushStyleColor(ImGuiCol_Button, theme.ColorToggleButtonChecked());
        PushStyleColor(ImGuiCol_ButtonHovered, theme.ColorToggleButtonCheckedHovered());
        PushStyleColor(ImGuiCol_ButtonActive, theme.ColorButtonActive());
        break;
    case ColorTheme::Window:
        PushStyleColor(ImGuiCol_WindowBg, theme.ColorWindowBackground());
        break;
    case ColorTheme::Overlay:
        PushStyleColor(ImGuiCol_WindowBg, theme.ColorOverlayBackground());
        break;
    }
}

void StyleColorsDrop::pop()
{
    ImGui::PopStyleColor();
}

void StyleColorsDrop::PushStyleColor(ImGuiCol idx, const ImVec4& col)
{
    ImGui::PushStyleColor(idx, col);
    incPopCount();
}

} // namespace context
