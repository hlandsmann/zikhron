// top include begin
#include <glad/glad.h>
// top include end
#include "Video.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <multimedia/MpvWrapper.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <ranges>
#include <utility>

namespace ranges = std::ranges;
namespace views = std::views;

namespace widget {
void Video::setup(std::shared_ptr<multimedia::MpvWrapper> _mpv)
{
    mpv = std::move(_mpv);

    ranges::generate(frames, genFrame);
    mpv->initGL();
}

Video::Video(const WidgetInit& init)
    : Widget{init}
{
    using namespace widget::layout;
    setExpandType(width_expand, height_expand);
}

void Video::displayTexture()
{
    auto tex = getActiveFrame()->tex;

    const auto& rect = getRect();
    textureWidth = static_cast<GLsizei>(rect.width);
    textureHeight = static_cast<GLsizei>(rect.height);

    auto* texture = reinterpret_cast<ImTextureID>(static_cast<intptr_t>(tex));
    ImGui::Image(texture, {rect.width, rect.height});
}

void Video::render()
{
    auto optScratchFrame = getScratchFrame();
    if (!optScratchFrame.has_value()) {
        spdlog::warn("NoVideoRender: optScratchFrame has no value");
        return;
    }

    auto& scratchFrame = **optScratchFrame;

    auto tex = scratchFrame.tex;
    auto fbo = scratchFrame.fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    scratchFrame.targetTime = mpv->render(fbo, textureWidth, textureHeight);
}

auto Video::calculateSize() const -> WidgetSize
{
    return {};
}

auto Video::getScratchFrame() -> std::optional<FrameArray::iterator>
{
    auto videoTime = mpv->getTime();
    auto count = ranges::count_if(frames, [videoTime](const Frame& frame) {
        return frame.targetTime < videoTime;
    });
    if (count < 2) {
        return {};
    }
    return {ranges::min_element(frames, std::ranges::less{}, &Frame::targetTime)};
}

auto Video::getActiveFrame() const -> FrameArray::const_iterator
{
    auto videoTime = mpv->getTime();
    auto oldFrames = frames | views::filter([videoTime](const Frame& frame) {
                         return frame.targetTime <= videoTime;
                     });
    if (oldFrames.empty()) {
        return frames.cbegin();
    }
    auto it = ranges::max_element(oldFrames,
                                  std::ranges::less{}, &Frame::targetTime);
    return ranges::find(frames, *it);
}

auto Video::genFrame() -> Frame
{
    GLuint tex = 0;
    GLuint fbo = 0;
    glGenFramebuffers(1, &fbo);
    glGenTextures(1, &tex);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return {.tex = tex, .fbo = fbo, .targetTime = 0};
}
} // namespace widget
