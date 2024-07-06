#pragma once
#include "ColorSet.h"
#include "Drop.h"
#include "Fonts.h"
#include "Texture.h"
#include "WidgetState.h"

#include <imgui.h>

namespace context {
class StyleVarsDrop;
class StyleColorsDrop;

enum class ColorTheme {
    ButtonDefault,
    ButtonChecked,
    Child,
    Window,
    Overlay,
};

class Theme
{ // clang-format off
    static constexpr ImVec4 s_colorButton                          = {0.25F, 0.25F, 0.25F, 1.0F};
    static constexpr ImVec4 s_colorButtonHovered                   = {0.4F, 0.4F, 0.4F, 1.0F};
    static constexpr ImVec4 s_colorButtonActive                    = {0.3F, 0.3F, 0.3F, 1.0F};
    static constexpr ImVec4 s_colorToggleButtonChecked             = {0.5F, 0.5F, 0.5F, 1.0F};
    static constexpr ImVec4 s_colorToggleButtonCheckedHovered      = {0.6F, 0.6F, 0.6F, 1.0F};
    static constexpr ImVec4 s_colorToggleButtonInsensitive         = {0.2F, 0.2F, 0.2F, 1.0F};
    static constexpr ImVec4 s_colorToggleButtonInsensitiveHovered  = {0.2F, 0.2F, 0.2F, 1.0F};

    static constexpr ImVec4 s_colorImage                    = {0.8F, 0.8F, 0.8F, 1.0F};
    static constexpr ImVec4 s_colorImageHovered             = {0.8F, 0.9F, 1.0F, 1.0F};
    static constexpr ImVec4 s_colorImageActive              = {1.0F, 1.0F, 1.0F, 1.0F};
    static constexpr ImVec4 s_colorImageChecked             = {0.9F, 0.9F, 0.9F, 1.0F};
    static constexpr ImVec4 s_colorImageCheckedHovered      = {0.9F, 1.0F, 1.0F, 1.0F};
    static constexpr ImVec4 s_colorImageInsensitive         = {0.6F, 0.6F, 0.6F, 1.0F};
    static constexpr ImVec4 s_colorImageInsensitiveHovered  = {0.6F, 0.6F, 0.6F, 1.0F};

    static constexpr ImVec4 s_colorBorder = {0.3F, 0.3F, 0.3F, 1.0F};
    static constexpr ImVec4 s_colorChildBackground = {0.15F, 0.15F, 0.15F, 1.0F};
    static constexpr ImVec4 s_colorWindowBackground = {0.17F, 0.17F, 0.17F, 1.0F};
    static constexpr ImVec4 s_colorOverlayBackground = {0.13F, 0.13F, 0.13F, 1.0F};
    // clang-format on

public:
    Theme(Fonts, Texture);

    [[nodiscard]] auto getColorSet() const -> const ColorSet&;

    [[nodiscard]] static auto dropImGuiStyleVars() -> StyleVarsDrop;
    [[nodiscard]] auto dropImGuiStyleColors(ColorTheme) const -> StyleColorsDrop;

    [[nodiscard]] auto ColorButton(WidgetState state) const -> const ImVec4&;
    [[nodiscard]] auto ColorButton() const -> const ImVec4&;
    [[nodiscard]] auto ColorButtonHovered() const -> const ImVec4&;
    [[nodiscard]] auto ColorButtonActive() const -> const ImVec4&;
    [[nodiscard]] auto ColorToggleButtonChecked() const -> const ImVec4&;
    [[nodiscard]] auto ColorToggleButtonCheckedHovered() const -> const ImVec4&;
    [[nodiscard]] auto ColorToggleButtonInsensitive() const -> const ImVec4&;
    [[nodiscard]] auto ColorToggleButtonInsensitiveHovered() const -> const ImVec4&;

    [[nodiscard]] auto ColorImage(WidgetState state) const -> const ImVec4&;
    [[nodiscard]] auto ColorImage() const -> const ImVec4&;
    [[nodiscard]] auto ColorImageHovered() const -> const ImVec4&;
    [[nodiscard]] auto ColorImageActive() const -> const ImVec4&;
    [[nodiscard]] auto ColorImageChecked() const -> const ImVec4&;
    [[nodiscard]] auto ColorImageCheckedHovered() const -> const ImVec4&;
    [[nodiscard]] auto ColorImageInsensitive() const -> const ImVec4&;
    [[nodiscard]] auto ColorImageInsensitiveHovered() const -> const ImVec4&;

    [[nodiscard]] auto ColorBorder() const -> const ImVec4&;
    [[nodiscard]] auto ColorChildBackground() const -> const ImVec4&;
    [[nodiscard]] auto ColorWindowBackground() const -> const ImVec4&;
    [[nodiscard]] auto ColorOverlayBackground() const -> const ImVec4&;

    [[nodiscard]] auto getFont() const -> const Fonts&;
    [[nodiscard]] auto getTexture() const -> const Texture&;

private:
    ImVec4 colorButton{s_colorButton};
    ImVec4 colorButtonHovered{s_colorButtonHovered};
    ImVec4 colorButtonActive{s_colorButtonActive};
    ImVec4 colorToggleButtonChecked{s_colorToggleButtonChecked};
    ImVec4 colorToggleButtonCheckedHovered{s_colorToggleButtonCheckedHovered};
    ImVec4 colorToggleButtonInsensitive{s_colorToggleButtonInsensitive};
    ImVec4 colorToggleButtonInsensitiveHovered{s_colorToggleButtonInsensitiveHovered};

    ImVec4 colorImage{s_colorImage};
    ImVec4 colorImageHovered{s_colorImageHovered};
    ImVec4 colorImageActive{s_colorImageActive};
    ImVec4 colorImageChecked{s_colorImageChecked};
    ImVec4 colorImageCheckedHovered{s_colorImageCheckedHovered};
    ImVec4 colorImageInsensitive{s_colorImageInsensitive};
    ImVec4 colorImageInsensitiveHovered{s_colorImageInsensitiveHovered};

    ImVec4 colorBorder{s_colorBorder};
    ImVec4 colorChildBackground{s_colorChildBackground};
    ImVec4 colorWindowBackground{s_colorWindowBackground};
    ImVec4 colorOverlayBackground{s_colorOverlayBackground};

    Fonts fonts;
    Texture texture;
    ColorSet colorSet;
};

class StyleVarsDrop : public Drop<StyleVarsDrop>
{
public:
    StyleVarsDrop();

private:
    friend class Drop<StyleVarsDrop>;
    static void pop();

    void PushStyleVar(ImGuiStyleVar idx, float val);
    void PushStyleVar(ImGuiStyleVar idx, const ImVec2& val);
};

class StyleColorsDrop : public Drop<StyleColorsDrop>
{
public:
    StyleColorsDrop(const Theme& theme, ColorTheme);

private:
    friend class Drop<StyleColorsDrop>;
    static void pop();
    void PushStyleColor(ImGuiCol idx, const ImVec4& col);
};
} // namespace context
