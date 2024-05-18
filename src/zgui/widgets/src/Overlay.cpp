#include "Overlay.h"

#include <Layer.h>
#include <TextToken.h>
#include <Window.h>
#include <context/Theme.h>
#include <context/imglog.h>
#include <imgui.h>

#include <algorithm>
#include <functional>
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

{
    layer = std::make_shared<widget::Layer>(widget::WidgetInit{
            .theme = getThemePtr(),
            .widgetIdGenerator = getWidgetIdGenerator(),
            .rect = std::make_shared<layout::Rect>(),
            .horizontalAlign = widget::layout::Align::start,
            .verticalAlign = widget::layout::Align::start,
            .expandTypeWidth = layout::width_expand,
            .expandTypeHeight = layout::width_expand,
            .parent = std::weak_ptr<widget::Widget>{}});
}

auto Overlay::dropOverlay(float x, float y) -> OverlayDrop
{
    auto layerSize = layer->getWidgetSize();
    auto rect = getRect();
    // auto layerSize = layer->getWidgetSizeFromRect(getLayerRect(rect));
    auto parentWindowRect = getParentWindowRect();
    // imglog::log("overlay in drop, x {}, y {}, w{}, h{}", rect.x, rect.y, rect.width, rect.height);
    rect.x = std::min(x + parentWindowRect.x, rect.width - layerSize.width);
    rect.y = std::min(y + parentWindowRect.y, rect.height - layerSize.height);
    rect.width = layerSize.width;
    rect.height = layerSize.height;

    imglog::log("overlay drop, x {}, y {}, w{}, h{}", rect.x, rect.y, rect.width, rect.height);

    if (ImGui::IsWindowFocused()) {
        if (firstDrop) {
            ImGui::SetNextWindowFocus();
            firstDrop = false;
        } else {
            closeNext = true;
        }
    }

    return {
            getName(),
            rect,
            getTheme().dropImGuiStyleColors(context::ColorTheme::Overlay),
            [this] {
                if (ImGui::IsWindowFocused()) {
                    firstDrop = false;
                }
            },
    };
}

void Overlay::setFirstDrop()
{
    firstDrop = true;
    closeNext = false;
}

auto Overlay::shouldClose() const -> bool
{
    return closeNext;
}

auto Overlay::calculateSize() const -> WidgetSize
{
    return layer->getWidgetSize();
}

auto Overlay::arrange(const layout::Rect& rect) -> bool
{
    // imglog::log("overlay arrange: w: {}, h: {}", rect.width, rect.height);
    setRect(rect);

    auto layerRect = getLayerRect(rect);
    // layerRect.height = layer->getWidgetSize().height;
    return layer->arrange(layerRect);
}

auto Overlay::getWidgetSizeFromRect(const layout::Rect& rect) -> WidgetSize
{
    const auto& layerRect = getLayerRect(rect);
    auto layerWidgetSize = layer->getWidgetSizeFromRect(layerRect);

    return {.width = std::max(rect.width, layerWidgetSize.width),
            .height = std::max(rect.height, layerWidgetSize.height)};
}

auto Overlay::getLayerRect(const layout::Rect& rect) const -> layout::Rect
{
    auto layerRect = layout::Rect{.x = 0.F,
                                  .y = 0.F,
                                  .width = std::min(rect.width, maxWidth),
                                  .height = rect.height};
    return layerRect;
}

auto Overlay::getParentWindowRect() const -> layout::Rect
{
    auto winParent = getParent();
    while (winParent) {
        if (auto* window = dynamic_cast<widget::Window*>(winParent.get())) {
            return window->getRect();
        }
        winParent = winParent->getParent();
    }
    return {};
}

OverlayDrop::OverlayDrop(const std::string& name, const widget::layout::Rect& rect,
                         context::StyleColorsDrop _styleColorsDrop,
                         std::function<void()> execAfterWindowOpen)
    : styleColorsDrop{std::move(_styleColorsDrop)}
{
    ImGui::SetNextWindowPos({rect.x, rect.y});
    ImGui::SetNextWindowSize({rect.width, rect.height});
    ImGui::Begin(name.c_str(), nullptr,
                 ImGuiWindowFlags_NoTitleBar
                         | ImGuiWindowFlags_NoMove
                         | ImGuiWindowFlags_NoResize);
    incPopCount();
    if (execAfterWindowOpen) {
        execAfterWindowOpen();
    }
}

void OverlayDrop::pop()
{
    ImGui::End();
}
} // namespace widget
