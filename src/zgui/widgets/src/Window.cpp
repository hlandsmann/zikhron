#include "Box.h"
#include "Widget.h"
#include <context/imglog.h>

#include <context/Theme.h>
#include <Window.h>
#include <imgui.h>

#include <memory>
#include <string>
#include <utility>

namespace widget {
Window::Window(std::shared_ptr<context::Theme> _theme,
               layout::Orientation _orientation,
               layout::Align _align,
               const std::shared_ptr<layout::Rect>& _rect,
               layout::SizeType _sizeTypeWidth, layout::SizeType _sizeTypeHeight,
               std::string _name)
    : Widget<Window>{std::move(_theme), _orientation, _align, _rect}
    , box{std::move(_theme), _orientation, _align, _rect}
    , sizeTypeWidth{_sizeTypeWidth}
    , sizeTypeHeight{_sizeTypeHeight}
    , name{std::move(_name)}
{}

auto Window::dropWindow() -> WindowDrop
{
    layout::Rect rect = Rect();
    layout::Rect layoutRect = {.x = 0, .y = 0, .width = rect.width, .height = rect.height};
    box.arrange(layoutRect);
    return {name, rect};
}

auto Window::getLayout() -> Box&
{
    return box;
}

auto Window::calculateSize() const -> WidgetSize
{
    auto size = box.getWidgetSize();
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