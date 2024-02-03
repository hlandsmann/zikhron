#include <WidgetIdGenerator.h>
#include <imgui.h>

namespace context {
auto WidgetIdGenerator::getNextId() -> WidgetId
{
    return WidgetId{id + 1};
}

WidgetIdDrop::WidgetIdDrop(WidgetId widgetId)
{
    ImGui::PushID(widgetId);
    incPopCount();
}

void WidgetIdDrop::pop()
{
    ImGui::PopID();
}

} // namespace context
