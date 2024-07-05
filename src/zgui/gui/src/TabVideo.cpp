#include "TabVideo.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <widgets/Layer.h>
#include <widgets/Window.h>

#include <memory>

namespace gui {
void TabVideo::setUp(std::shared_ptr<widget::Layer> layer)
{
    using namespace widget::layout;
    auto& cardWindow = *layer->add<widget::Window>(Align::start, width_expand, height_expand, "card_text");
    windowId = cardWindow.getWidgetId();
}

void TabVideo::displayOnLayer(widget::Layer& layer)
{
    auto window = layer.getWidget<widget::Window>(windowId);
    auto droppedWindow = window.dropWindow();
    // ImGui::PushStyleColor(ImGuiCol_ChildBg, 0xFFFFFFFF);
    // ImGui::BeginChild("##tetpla", {100, 100});
    // ImGui::Text("Hellasdfasdfasdfasdfasdfasdfasdfasdf");
    // ImGui::Button("ClickMe");
    // ImDrawList* draw_list = ImGui::GetWindowDrawList();
    // ImGui::RenderTextEllipsis(draw_list,
    //     {50,50},
    //     {100, 150},
    //     150,
    //     150,
    //     "asdf;lsdf;lsdf;lsdf;lsdf;lsdf;lsdf;lsdf;lsdf;lsdf;l",
    //     nullptr,nullptr);
    // const ImVec2 *text_size_if_known)
    // ImGui::EndChild();
    // ImGui::PopStyleColor();
}
} // namespace gui
