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
class MpvWrapper : public std::enable_shared_from_this<MpvWrapper>
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
    void play();
    void pause();

    void playFrom(double start);
    void setFragment(double start, double end);
    void seek(double pos);
    void setSubtitle(bool enabled);
    [[nodiscard]] auto getDuration() const -> double;
    [[nodiscard]] auto getTimePos() const -> double;
    [[nodiscard]] auto is_playing() const -> bool;

    void initGL();
    auto render(GLuint fbo, int width, int height) -> int64_t;
    [[nodiscard]] auto getTime() const -> int64_t;

    // kokoro Signals:
    [[nodiscard]] auto SignalShouldRender() const -> kocoro::VolatileSignal<bool>&;
    [[nodiscard]] auto SignalTimePos() const -> kocoro::VolatileSignal<double>&;

private:
    enum class CommandType {
        seek,
        stoppedMediaSeek,
    };

    struct Command
    {
        CommandType type{};
        double seekPosition{};
    };
    auto handleEventTask() -> kocoro::Task<>;
    auto handleCommandTask() -> kocoro::Task<>;
    void mpvCommandSeek(double pos);
    void mpvCommandPlay(bool play);
    void setSeekCommandTask(double pos, CommandType commandType);

    void handle_mpv_event(mpv_event* event);
    [[nodiscard]] auto getNextFrameTargetTime() const -> int64_t;

    static void onMpvUpdate(void* mpv);

    void resetTime();

    std::function<void(mpv_handle*)> mpv_deleter = [](mpv_handle* mpvHandle) {
        mpv_terminate_destroy(mpvHandle);
    };
    std::unique_ptr<mpv_handle, decltype(mpv_deleter)> mpv;

    std::function<void(mpv_render_context*)> renderCtx_deleter = [](mpv_render_context* render_ctx) {
        mpv_render_context_free(render_ctx);
    };
    std::unique_ptr<mpv_render_context, decltype(renderCtx_deleter)> renderCtx;
    std::string mediaFile;

    int mpv_flag_paused{1}; // integer shared with mpv

    bool isSeeking{false};
    bool paused{false};
    bool shouldUnpause{false};
    double stopAtPosition{0};
    double volume{100.F};
    double duration{};
    double timePos{};

    bool stopped{true};


    // Signals:
    std::shared_ptr<kocoro::VolatileSignal<bool>>
            signalShouldRender;
    std::shared_ptr<kocoro::VolatileSignal<double>> signalTimePos;
    std::shared_ptr<kocoro::VolatileSignal<double>> signalTimePosInternal;
    std::shared_ptr<kocoro::VolatileSignal<bool>> signalEvent;
    std::shared_ptr<kocoro::VolatileSignal<Command>> signalCommand;
    std::shared_ptr<kocoro::PersistentSignal<double>> signalDuration; // a file is opened if this is duration available
};
} // namespace multimedia
