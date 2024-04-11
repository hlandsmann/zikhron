#include "Overlay.h"

#include <TextToken.h>
#include <context/Theme.h>
#include <imgui.h>

#include <memory>
#include <string>
#include <utility>

namespace widget {

void Overlay::setup(float _maxWidth)
{
    using namespace widget::layout;
    setExpandType(width_expand, height_expand);

    maxWidth = _maxWidth;
}

Overlay::Overlay(WidgetInit _init)
    : Widget{std::move(_init)}

{}

OverlayDrop::OverlayDrop(const std::string& name, const widget::layout::Rect& rect,
                         context::StyleColorsDrop _styleColorsDrop)
    : styleColorsDrop{std::move(_styleColorsDrop)}
{
    ImGui::SetNextWindowPos({rect.x, rect.y});
    ImGui::SetNextWindowSize({rect.width, rect.height});
    ImGui::Begin(name.c_str(), nullptr,
                 ImGuiWindowFlags_NoTitleBar
                         | ImGuiWindowFlags_NoMove
                         | ImGuiWindowFlags_NoResize);
    incPopCount();
}

void OverlayDrop::pop()
{
    ImGui::End();
}
} // namespace widget
