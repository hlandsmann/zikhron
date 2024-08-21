#pragma once
#include <GL/gl.h>
#include <mpv/client.h>
#include <mpv/render.h>
#include <mpv/render_gl.h>

#include <cstdint>
#include <filesystem>
#include <functional>
#include <kocoro/kocoro.hpp>
#include <memory>
#include <string>

namespace multimedia {
class MpvWrapper
{
public:
    MpvWrapper(std::shared_ptr<kocoro::SynchronousExecutor> executor);
    virtual ~MpvWrapper() = default;

    MpvWrapper(const MpvWrapper&) = delete;
    MpvWrapper(MpvWrapper&&) = delete;
    auto operator=(const MpvWrapper&) -> MpvWrapper& = delete;
    auto operator=(MpvWrapper&&) -> MpvWrapper& = delete;

    void openFile(const std::filesystem::path& mediaFile);
    [[nodiscard]] auto getMediaFile() const -> std::filesystem::path;
    void play(double until = 0.0);
    void playFrom(double start);
    void playFragment(double start, double end);
    void pause();
    void seek(double pos);
    [[nodiscard]] auto getDuration() const -> double;
    [[nodiscard]] auto getTimePos() const -> double;

    [[nodiscard]] auto is_paused() const -> bool { return paused; }

    void initGL();
    auto render(GLuint fbo, int width, int height) -> int64_t;
    [[nodiscard]] auto getTime() const -> int64_t;

    // kokoro Signals:
    [[nodiscard]] auto SignalShouldRender() const -> kocoro::VolatileSignal<bool>&;
    [[nodiscard]] auto SignalTimePos() const -> kocoro::VolatileSignal<double>&;

private:
    auto handleEventTask() -> kocoro::Task<>;

    void handle_mpv_event(mpv_event* event);
    [[nodiscard]] auto getNextFrameTargetTime() const -> int64_t;

    static void onMpvUpdate(void* mpv);

    void closeFile();

    std::function<void(mpv_handle*)> mpv_deleter = [](mpv_handle* mpvHandle) {
        mpv_terminate_destroy(mpvHandle);
    };
    std::unique_ptr<mpv_handle, decltype(mpv_deleter)> mpv;

    std::function<void(mpv_render_context*)> renderCtx_deleter = [](mpv_render_context* render_ctx) {
        mpv_render_context_free(render_ctx);
    };
    std::unique_ptr<mpv_render_context, decltype(renderCtx_deleter)> renderCtx;
    std::string mediaFile;

    int mpv_flag_paused = 1;
    bool paused{false};
    double stopAtPosition = 0;
    double volume{100.F};
    double duration{};
    double timePos{};

    // Signals:
    std::shared_ptr<kocoro::VolatileSignal<bool>> signalShouldRender;
    std::shared_ptr<kocoro::VolatileSignal<double>> signalTimePos;
    std::shared_ptr<kocoro::VolatileSignal<bool>> signalEvent;
};
} // namespace multimedia
