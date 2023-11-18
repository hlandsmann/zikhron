#include "Theme.h"

#include <imgui.h>
namespace widget {
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
} // namespace widget
