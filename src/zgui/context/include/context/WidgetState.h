#pragma once

namespace context {

enum class WidgetState {
    default_state,
    hovered,
    active,
    checked,
    checked_hovered,
    insensitive,
    insensitive_hovered
};

auto getWidgetState(bool disabled, bool enabled) -> WidgetState;

} // namespace context
