#include <TextDraw.h>
#include <string>

#include <imgui.h>
#include <imgui_internal.h>
namespace ImGui {
void TextWithHoverColor(ImVec4 col, const std::string& fmt)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems) {
        return;
    }

    // Format text

    // Layout
    const ImVec2 text_pos(window->DC.CursorPos.x, window->DC.CursorPos.y + window->DC.CurrLineTextBaseOffset);
    const ImVec2 text_size = CalcTextSize(fmt.cbegin().base(), fmt.cend().base());
    ImRect bb(text_pos.x, text_pos.y, text_pos.x + text_size.x, text_pos.y + text_size.y);
    ItemSize(text_size, 0.0F);
    if (!ItemAdd(bb, 0)) {
        return;
    }

    // Render
    bool hovered = IsItemHovered();
    if (hovered) {
        PushStyleColor(ImGuiCol_Text, col);
    }
    RenderText(bb.Min, fmt.cbegin().base(), fmt.cend().base(), false);
    if (hovered) {
        PopStyleColor();
    }
}
} // namespace ImGui
