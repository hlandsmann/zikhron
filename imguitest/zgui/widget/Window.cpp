#include "Window.h"

#include "Layout.h"
#include "Widget.h"
#include "imglog.h"

#include <imgui.h>

#include <memory>
#include <string>
#include <utility>

namespace widget {
Window::Window(layout::Align _align, layout::Orientation _orientation, const std::shared_ptr<layout::Rect>& _rect,
               layout::SizeType _sizeTypeWidth, layout::SizeType _sizeTypeHeight,
               std::string _name)
    : Widget<Window>{_align, _orientation, std::move(_rect)}
    , layout{_align, _orientation, _rect}
    , sizeTypeWidth{_sizeTypeWidth}
    , sizeTypeHeight{_sizeTypeHeight}
    , name{std::move(_name)}
{}

auto Window::dropWindow() -> WindowDrop
{
    layout::Rect rect = Rect();
    layout::Rect layoutRect = {.x = 0, .y = 0, .width = rect.width, .height = rect.height};
    layout.arrange(layoutRect);
    return {name, rect};
}

auto Window::getLayout() -> Layout&
{
    return layout;
}

auto Window::calculateSize() const -> WidgetSize
{
    auto size = layout.getWidgetSize();
    return {.widthType = sizeTypeWidth,
            .heightType = sizeTypeHeight,
            .width = size.width,
            .height = size.height};
}

WindowDrop::WindowDrop(const std::string& name, const widget::layout::Rect& rect)
{
    ImGui::SetNextWindowPos({rect.x, rect.y});
    ImGui::SetNextWindowSize({rect.width, rect.height});
    imglog::log("name: {}, x: {}, y: {}, w: {}, h: {}", name, rect.x, rect.y, rect.width, rect.height);
    ImGui::Begin(name.c_str(), nullptr,
                 ImGuiWindowFlags_NoTitleBar
                         | ImGuiWindowFlags_NoMove
                         | ImGuiWindowFlags_NoResize);
}

WindowDrop::~WindowDrop()
{
    ImGui::End();
}
} // namespace widget
