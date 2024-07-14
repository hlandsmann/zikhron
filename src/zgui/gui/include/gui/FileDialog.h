#pragma once
#include <widgets/detail/Widget.h>

#include <filesystem>
#include <kocoro/kocoro.hpp>
#include <memory>

namespace gui {

class FileDialog
{
public:
    FileDialog(widget::WidgetSize widgetSize,
               std::filesystem::path directory,
               std::shared_ptr<kocoro::VolatileSignal<std::filesystem::path>> fileOpen);
    FileDialog(const FileDialog&) = delete;
    FileDialog(FileDialog&&) = delete;
    auto operator=(const FileDialog&) -> FileDialog& = delete;
    auto operator=(FileDialog&&) -> FileDialog& = delete;
    virtual ~FileDialog() = default;
    void draw();

private:
    widget::WidgetSize widgetSize;
    std::shared_ptr<kocoro::VolatileSignal<std::filesystem::path>> fileOpen;
};

} // namespace gui
