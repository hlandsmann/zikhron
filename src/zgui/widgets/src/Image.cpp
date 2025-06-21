#include "Image.h"

#include <imgui.h>

#include <filesystem>
#include <utility>

namespace widget {
void Image::setup(const std::filesystem::path& imageFile)
{
    texture = context::Texture::loadTextureFromFile(imageFile);
}

Image::Image(WidgetInit init)
    : Widget{std::move(init)}
{}

void Image::draw() const
{
    const auto& rect = getRect();
    ImGui::SetCursorPos({rect.x, rect.y});
    ImGui::Image(reinterpret_cast<ImTextureID>(texture.data), ImVec2(texture.width, texture.height));
}

auto Image::calculateSize() const -> WidgetSize
{
    return {.width = texture.width,
            .height = texture.height};
}
} // namespace widget
