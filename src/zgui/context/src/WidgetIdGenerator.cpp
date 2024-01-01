#include <WidgetIdGenerator.h>
#include <imgui.h>

namespace context {
auto WidgetIdGenerator::getNextId() -> int
{
    return id++;
}

WidgetIdDrop::WidgetIdDrop(int widgetId)
{
    ImGui::PushID(widgetId);
    incPopCount();
}

void WidgetIdDrop::pop()
{
    ImGui::PopID();
}

} // namespace context
