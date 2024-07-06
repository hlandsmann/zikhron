#include "TabVideo.h"

#include <GroupAdd.h>
#include <ImGuiFileDialog/ImGuiFileDialog.h>
#include <imgui.h>
#include <widgets/Layer.h>
#include <widgets/Window.h>

#include <memory>

namespace gui {
void TabVideo::setUp(std::shared_ptr<widget::Layer> layer)
{
    using namespace widget::layout;
    auto& cardWindow = *layer->add<widget::Window>(Align::start, width_expand, height_expand, "card_text");
    windowId = cardWindow.getWidgetId();
    auto grid = cardWindow.add<widget::Grid>(Align::start, gridCfg, 4, widget::Grid::Priorities{0.25F, 0.25F, 0.25F, 0.25F});
    groupAdd = std::make_unique<GroupAdd>(grid);
}

void TabVideo::displayOnLayer(widget::Layer& layer)
{
    auto window = layer.getWidget<widget::Window>(windowId);
    auto droppedWindow = window.dropWindow();
    ;

    // open Dialog Simple
    if (groupAdd->draw()) {
        IGFD::FileDialogConfig config;
        config.path = ".";
        config.flags = ImGuiFileDialogFlags_Modal;
        ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".cpp,.h,.hpp", config);
    }

    // display
    if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) {
        // action if OK
        if (ImGuiFileDialog::Instance()->IsOk()) {
            std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
            spdlog::warn("open: {}, {}", filePath, filePathName);
            // action
        }

        // close
        ImGuiFileDialog::Instance()->Close();
    }

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
