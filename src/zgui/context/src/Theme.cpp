#include <Fonts.h>
#include <Texture.h>
#include <Theme.h>
#include <WidgetState.h>
#include <imgui.h>

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

auto Theme::dropImGuiStyleColors() const -> StyleColorsDrop
{
    return {*this};
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
    case WidgetState::enabled:
        return ColorToggleButtonEnabled();
    case WidgetState::enabled_hovered:
        return ColorToggleButtonEnabledHovered();
    case WidgetState::disabled:
        return ColorToggleButtonDisabled();
    case WidgetState::disabled_hovered:
        return ColorToggleButtonDisabledHovered();
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

auto Theme::ColorImage(WidgetState state) const -> const ImVec4&
{
    switch (state) {
    case WidgetState::default_state:
        return ColorImage();
    case WidgetState::hovered:
        return ColorImageHovered();
    case WidgetState::active:
        return ColorImageActive();
    case WidgetState::enabled:
        return ColorImageEnabled();
    case WidgetState::enabled_hovered:
        return ColorImageEnabledHovered();
    case WidgetState::disabled:
        return ColorImageDisabled();
    case WidgetState::disabled_hovered:
        return ColorImageDisabledHovered();
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

StyleVarsDrop::StyleVarsDrop()
{
    PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0F, 0.0F));
    PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0F, 0.0F));
}

StyleVarsDrop::~StyleVarsDrop()
{
    ImGui::PopStyleVar(countStyleVars);
}

void StyleVarsDrop::PushStyleVar(ImGuiStyleVar idx, float val)
{
    ImGui::PushStyleVar(idx, val);
    countStyleVars++;
}

void StyleVarsDrop::PushStyleVar(ImGuiStyleVar idx, const ImVec2& val)
{
    ImGui::PushStyleVar(idx, val);
    countStyleVars++;
}

StyleColorsDrop::StyleColorsDrop(const Theme& theme)
{
    PushStyleColor(ImGuiCol_Button, theme.ColorButton());
    PushStyleColor(ImGuiCol_ButtonHovered, theme.ColorButtonHovered());
    PushStyleColor(ImGuiCol_ButtonActive, theme.ColorButtonActive());
}

StyleColorsDrop::~StyleColorsDrop()
{
    ImGui::PopStyleColor(countStyleColors);
}

void StyleColorsDrop::PushStyleColor(ImGuiCol idx, const ImVec4& col)
{
    ImGui::PushStyleColor(idx, col);
    countStyleColors++;
}

} // namespace context
