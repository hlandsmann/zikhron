#pragma once
#include "detail/Widget.h" // IWYU pragma: export detail/Widget.h

#include <GL/gl.h>
#include <multimedia/MpvWrapper.h>

#include <array>
#include <cstdint>
#include <kocoro/kocoro.hpp>
#include <memory>
#include <optional>

namespace widget {
class Video : public Widget
{
public:
    void setup(std::shared_ptr<multimedia::MpvWrapper> mpv);
    Video(const WidgetInit& init);
    ~Video() override = default;

    Video(const Video&) = delete;
    Video(Video&&) = delete;
    auto operator=(const Video&) -> Video& = delete;
    auto operator=(Video&&) -> Video& = delete;

    void displayTexture();
    void render();

protected:
    auto calculateSize() const -> WidgetSize override;

private:
    struct Frame
    {
        GLuint tex{};
        GLuint fbo{};
        int64_t targetTime{};
        auto operator==(const Frame&) const -> bool = default;
    };

    using FrameArray = std::array<Frame, 3>;
    FrameArray frames;

    [[nodiscard]] auto getScratchFrame() -> std::optional<FrameArray::iterator>;
    [[nodiscard]] auto getActiveFrame() const -> FrameArray::const_iterator;
    [[nodiscard]] static auto genFrame() -> Frame;

    int textureWidth{};
    int textureHeight{};
    std::shared_ptr<multimedia::MpvWrapper> mpv;
    // std::shared_ptr<ImPlay::Mpv> mpv;
    std::shared_ptr<kocoro::VolatileSignal<bool>> signalShouldRender;
};
} // namespace widget
