#pragma once
#include "Fonts.h"
#include "Texture.h"
#include "WidgetState.h"

#include <imgui.h>

#include <array>

namespace context {
class StyleVarsDrop;
class StyleColorsDrop;

enum class ColorTheme {
    ButtonDefault,
    Window,
};

constexpr auto bToColor(unsigned char r, unsigned char g, unsigned char b) -> ImVec4
{
    return {static_cast<float>(r) / 255, static_cast<float>(g) / 255, static_cast<float>(b) / 255, 1.F};
}

class Theme
{ // clang-format off
    static constexpr ImVec4 s_colorButton                          = {0.2F, 0.2F, 0.2F, 1.0F};
    static constexpr ImVec4 s_colorButtonHovered                   = {0.3F, 0.3F, 0.3F, 1.0F};
    static constexpr ImVec4 s_colorButtonActive                    = {0.5F, 0.5F, 0.5F, 1.0F};
    static constexpr ImVec4 s_colorToggleButtonChecked             = {0.5F, 0.5F, 0.5F, 1.0F};
    static constexpr ImVec4 s_colorToggleButtonCheckedHovered      = {0.6F, 0.6F, 0.6F, 1.0F};
    static constexpr ImVec4 s_colorToggleButtonInsensitive         = {0.2F, 0.2F, 0.2F, 1.0F};
    static constexpr ImVec4 s_colorToggleButtonInsensitiveHovered  = {0.3F, 0.3F, 0.3F, 1.0F};

    static constexpr ImVec4 s_colorImage                    = {0.8F, 0.8F, 0.8F, 1.0F};
    static constexpr ImVec4 s_colorImageHovered             = {0.8F, 0.9F, 1.0F, 1.0F};
    static constexpr ImVec4 s_colorImageActive              = {1.0F, 1.0F, 1.0F, 1.0F};
    static constexpr ImVec4 s_colorImageChecked             = {0.9F, 0.9F, 0.9F, 1.0F};
    static constexpr ImVec4 s_colorImageCheckedHovered      = {0.9F, 1.0F, 1.0F, 1.0F};
    static constexpr ImVec4 s_colorImageInsensitive         = {0.6F, 0.6F, 0.6F, 1.0F};
    static constexpr ImVec4 s_colorImageInsensitiveHovered  = {0.6F, 0.6F, 0.6F, 1.0F};

    static constexpr ImVec4 s_colorWindowBackground = {0.15F, 0.15F, 0.15F, 1.0F};
    // clang-format on
    static constexpr std::array<ImVec4, 11> colors = {bToColor(0xff, 0xe1, 0x19),
                                                      bToColor(0x3c, 0xd4, 0x4b),
                                                      bToColor(0x42, 0xd4, 0xf4),
                                                      bToColor(0xf5, 0x82, 0x31),
                                                      bToColor(0xf0, 0x32, 0xe6),
                                                      bToColor(0xfa, 0xbe, 0xd4),
                                                      bToColor(0x91, 0x3e, 0xc4),
                                                      bToColor(0xff, 0x29, 0x4B),
                                                      bToColor(0x43, 0x63, 0xff),
                                                      bToColor(0xbf, 0xef, 0x45),
                                                      bToColor(0x46, 0x99, 0x90)};

    public : Theme(Fonts, Texture);

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

    [[nodiscard]] auto ColorWindowBackground() const -> const ImVec4&;

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

    ImVec4 colorWindowBackground{s_colorWindowBackground};

    Fonts fonts;
    Texture texture;
};

class StyleVarsDrop
{
public:
    StyleVarsDrop();
    ~StyleVarsDrop();

    StyleVarsDrop(const StyleVarsDrop&) = delete;
    StyleVarsDrop(StyleVarsDrop&&) noexcept;
    auto operator=(const StyleVarsDrop&) -> StyleVarsDrop& = delete;
    auto operator=(StyleVarsDrop&&) noexcept -> StyleVarsDrop&;

private:
    void PushStyleVar(ImGuiStyleVar idx, float val);
    void PushStyleVar(ImGuiStyleVar idx, const ImVec2& val);

    int countStyleVars{0};
};

class StyleColorsDrop
{
public:
    StyleColorsDrop(const Theme& theme, ColorTheme);
    ~StyleColorsDrop();

    StyleColorsDrop(const StyleColorsDrop&) = delete;
    StyleColorsDrop(StyleColorsDrop&&) noexcept;
    auto operator=(const StyleColorsDrop&) -> StyleColorsDrop& = delete;
    auto operator=(StyleColorsDrop&&) noexcept -> StyleColorsDrop&;

private:
    void PushStyleColor(ImGuiCol idx, const ImVec4& col);
    int countStyleColors{0};
};
} // namespace context
