#include "FileDialog.h"

#include <ImGuiFileDialog/ImGuiFileDialog.h>
#include <imgui.h>
#include <spdlog/spdlog.h>
#include <widgets/detail/Widget.h>

#include <filesystem>
#include <kocoro/kocoro.hpp>
#include <memory>
#include <string>
#include <utility>

namespace gui {

FileDialog::FileDialog(widget::WidgetSize _widgetSize,
                       std::filesystem::path directory,
                       std::shared_ptr<kocoro::VolatileSignal<std::filesystem::path>> _fileOpen)
    : widgetSize{_widgetSize}
    , fileOpen{std::move(_fileOpen)}
{
    IGFD::FileDialogConfig config;
    config.path = directory.string();
    config.flags = ImGuiFileDialogFlags_Modal;
    ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".((mp4|mkv))", config);
}

void FileDialog::draw()
{
    if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey", ImGuiWindowFlags_NoCollapse,
                                             {widgetSize.width - 20, widgetSize.height - 20},
                                             {widgetSize.width - 20, widgetSize.height - 20})) {
        // action if OK
        if (ImGuiFileDialog::Instance()->IsOk()) {
            std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
            spdlog::warn("open: {}, {}", filePath, filePathName);
            fileOpen->set(filePathName);
            // action
        } else {
            fileOpen->set({});
        }

        // close
        ImGuiFileDialog::Instance()->Close();
    }
}

} // namespace gui
