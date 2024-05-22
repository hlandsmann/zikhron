#include <WidgetState.h>
#include <imgui.h>

namespace context {
auto getWidgetState(bool sensitive, bool checked) -> WidgetState
{
    bool hovered = ImGui::IsItemHovered();
    // bool active = hovered && ImGui::IsMouseDown(0) ;
    bool active = ImGui::IsItemActive();
    if (!sensitive) {
        if (hovered) {
            return WidgetState::insensitive_hovered;
        }
        return WidgetState::insensitive;
    }
    if (active) {
        return WidgetState::active;
    }
    if (checked) {
        if (hovered) {
            return WidgetState::checked_hovered;
        }
        return WidgetState::checked;
    }
    if (hovered) {
        return WidgetState::hovered;
    }

    return WidgetState::default_state;
}
} // namespace context
