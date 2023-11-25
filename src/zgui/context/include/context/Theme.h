#pragma once
#include "Fonts.h"
#include "Texture.h"
#include "WidgetState.h"

#include <imgui.h>

namespace context {
class StyleVarsDrop;
class StyleColorsDrop;
class Theme
{ // clang-format off
    static constexpr ImVec4 s_colorButton                       = {0.2F, 0.2F, 0.2F, 1.0F};
    static constexpr ImVec4 s_colorButtonHovered                = {0.3F, 0.3F, 0.3F, 1.0F};
    static constexpr ImVec4 s_colorButtonActive                 = {0.5F, 0.5F, 0.5F, 1.0F};
    static constexpr ImVec4 s_colorToggleButtonEnabled          = {0.5F, 0.5F, 0.5F, 1.0F};
    static constexpr ImVec4 s_colorToggleButtonEnabledHovered   = {0.6F, 0.6F, 0.6F, 1.0F};
    static constexpr ImVec4 s_colorToggleButtonDisabled         = {0.2F, 0.2F, 0.2F, 1.0F};
    static constexpr ImVec4 s_colorToggleButtonDisabledHovered  = {0.3F, 0.3F, 0.3F, 1.0F};

    static constexpr ImVec4 s_colorImage                 = {1.0F, 1.0F, 1.0F, 1.0F};
    static constexpr ImVec4 s_colorImageHovered          = {0.4F, 0.8F, 1.0F, 1.0F};
    static constexpr ImVec4 s_colorImageActive           = {1.0F, 1.0F, 1.0F, 1.0F};
    static constexpr ImVec4 s_colorImageEnabled          = {1.0F, 1.0F, 1.0F, 1.0F};
    static constexpr ImVec4 s_colorImageEnabledHovered   = {0.4F, 0.8F, 1.0F, 1.0F};
    static constexpr ImVec4 s_colorImageDisabled         = {0.6F, 0.6F, 0.6F, 1.0F};
    static constexpr ImVec4 s_colorImageDisabledHovered  = {0.4F, 1.0F, 1.0F, 1.0F};
    // clang-format on
public:
    Theme(Fonts, Texture);

    [[nodiscard]] static auto dropImGuiStyleVars() -> StyleVarsDrop;
    [[nodiscard]] auto dropImGuiStyleColors() const -> StyleColorsDrop;

    [[nodiscard]] auto ColorButton(WidgetState state) const -> const ImVec4&;
    [[nodiscard]] auto ColorButton() const -> const ImVec4&;
    [[nodiscard]] auto ColorButtonHovered() const -> const ImVec4&;
    [[nodiscard]] auto ColorButtonActive() const -> const ImVec4&;
    [[nodiscard]] auto ColorToggleButtonEnabled() const -> const ImVec4&;
    [[nodiscard]] auto ColorToggleButtonEnabledHovered() const -> const ImVec4&;
    [[nodiscard]] auto ColorToggleButtonDisabled() const -> const ImVec4&;
    [[nodiscard]] auto ColorToggleButtonDisabledHovered() const -> const ImVec4&;

    [[nodiscard]] auto ColorImage(WidgetState state) const -> const ImVec4&;
    [[nodiscard]] auto ColorImage() const -> const ImVec4&;
    [[nodiscard]] auto ColorImageHovered() const -> const ImVec4&;
    [[nodiscard]] auto ColorImageActive() const -> const ImVec4&;
    [[nodiscard]] auto ColorImageEnabled() const -> const ImVec4&;
    [[nodiscard]] auto ColorImageEnabledHovered() const -> const ImVec4&;
    [[nodiscard]] auto ColorImageDisabled() const -> const ImVec4&;
    [[nodiscard]] auto ColorImageDisabledHovered() const -> const ImVec4&;

    [[nodiscard]] auto getFont() const -> const Fonts&;
    [[nodiscard]] auto getTexture() const -> const Texture&;

private:
    ImVec4 colorButton{s_colorButton};
    ImVec4 colorButtonHovered{s_colorButtonHovered};
    ImVec4 colorButtonActive{s_colorButtonActive};
    ImVec4 colorToggleButtonEnabled{s_colorToggleButtonEnabled};
    ImVec4 colorToggleButtonEnabledHovered{s_colorToggleButtonEnabledHovered};
    ImVec4 colorToggleButtonDisabled{s_colorToggleButtonDisabled};
    ImVec4 colorToggleButtonDisabledHovered{s_colorToggleButtonDisabledHovered};

    ImVec4 colorImage{s_colorImage};
    ImVec4 colorImageHovered{s_colorImageHovered};
    ImVec4 colorImageActive{s_colorImageActive};
    ImVec4 colorImageEnabled{s_colorImageEnabled};
    ImVec4 colorImageEnabledHovered{s_colorImageEnabledHovered};
    ImVec4 colorImageDisabled{s_colorImageDisabled};
    ImVec4 colorImageDisabledHovered{s_colorImageDisabledHovered};

    Fonts fonts;
    Texture texture;
};

class StyleVarsDrop
{
public:
    StyleVarsDrop();
    ~StyleVarsDrop();

    StyleVarsDrop(const StyleVarsDrop&) = delete;
    StyleVarsDrop(StyleVarsDrop&&) = default;
    auto operator=(const StyleVarsDrop&) -> StyleVarsDrop& = delete;
    auto operator=(StyleVarsDrop&&) -> StyleVarsDrop& = default;

private:
    void PushStyleVar(ImGuiStyleVar idx, float val);
    void PushStyleVar(ImGuiStyleVar idx, const ImVec2& val);

    int countStyleVars{0};
};

class StyleColorsDrop
{
public:
    StyleColorsDrop(const Theme& theme);
    ~StyleColorsDrop();

    StyleColorsDrop(const StyleColorsDrop&) = delete;
    StyleColorsDrop(StyleColorsDrop&&) = default;
    auto operator=(const StyleColorsDrop&) -> StyleColorsDrop& = delete;
    auto operator=(StyleColorsDrop&&) -> StyleColorsDrop& = default;

private:
    void PushStyleColor(ImGuiCol idx, const ImVec4& col);
    int countStyleColors{0};
};
} // namespace context
