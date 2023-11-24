#pragma once

namespace context {

enum class WidgetState {
    default_state,
    hovered,
    active,
    enabled,
    enabled_hovered,
    disabled,
    disabled_hovered
};

auto getWidgetState(bool disabled, bool enabled) -> WidgetState;

} // namespace context
