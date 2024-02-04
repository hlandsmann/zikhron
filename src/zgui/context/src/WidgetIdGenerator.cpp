#include <WidgetIdGenerator.h>
#include <imgui.h>

namespace context {
auto WidgetIdGenerator::getNextId() -> WidgetId
{
    id = WidgetId{id + 1};
    return id;
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
