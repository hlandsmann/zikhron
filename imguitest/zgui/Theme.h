#pragma once
#include <imgui.h>

class Theme
{ // clang-format off
    static constexpr ImVec4 s_colorButton         = {0.2F, 0.2F, 0.2F, 1.0F};
    static constexpr ImVec4 s_colorButtonHovered  = {0.3F, 0.3F, 0.3F, 1.0F};
    static constexpr ImVec4 s_colorButtonActive   = {0.5F, 0.5F, 0.5F, 1.0F};

    static constexpr ImVec4 s_colorToggleButtonEnabled          = {0.5F, 0.5F, 0.5F, 1.0F};
    static constexpr ImVec4 s_colorToggleButtonEnabledHovered   = {0.6F, 0.6F, 0.6F, 1.0F};
    static constexpr ImVec4 s_colorToggleButtonDisabled         = {0.2F, 0.2F, 0.2F, 1.0F};
    static constexpr ImVec4 s_colorToggleButtonDisabledHovered  = {0.3F, 0.3F, 0.3F, 1.0F};
    // clang-format on
public:
    Theme() = default;

    [[nodiscard]] auto ColorButton() const -> const ImVec4&;
    [[nodiscard]] auto ColorButtonHovered() const -> const ImVec4&;
    [[nodiscard]] auto ColorButtonActive() const -> const ImVec4&;
    [[nodiscard]] auto ColorToggleButtonEnabled() const -> const ImVec4&;
    [[nodiscard]] auto ColorToggleButtonEnabledHovered() const -> const ImVec4&;
    [[nodiscard]] auto ColorToggleButtonDisabled() const -> const ImVec4&;
    [[nodiscard]] auto ColorToggleButtonDisabledHovered() const -> const ImVec4&;

private:
    ImVec4 colorButton{s_colorButton};
    ImVec4 colorButtonHovered{s_colorButtonHovered};
    ImVec4 colorButtonActive{s_colorButtonActive};
    ImVec4 colorToggleButtonEnabled{s_colorToggleButtonEnabled};
    ImVec4 colorToggleButtonEnabledHovered{s_colorToggleButtonEnabledHovered};
    ImVec4 colorToggleButtonDisabled{s_colorToggleButtonDisabled};
    ImVec4 colorToggleButtonDisabledHovered{s_colorToggleButtonDisabledHovered};
};
