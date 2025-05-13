// clang-format off
#include <epoxy/glx.h>
#include <epoxy/egl.h>
#include <epoxy/gl_generated.h>
// clang-format on
#include <GL/gl.h>
#include <GL/glext.h>
#include <GLFW/glfw3.h>
#include <MpvWrapper.h>
#include <mpv/client.h>
#include <mpv/render.h>
#include <mpv/render_gl.h>
#include <spdlog/spdlog.h>

#include <bit>
#include <cstdint>
#include <filesystem>
#include <kocoro/kocoro.hpp>
#include <memory>
#include <stdexcept>
#include <string>

#ifdef GDK_WINDOWING_X11
#include <epoxy/glx.h>
#include <gdk/x11/gdkx.h>
#endif
#ifdef GDK_WINDOWING_WAYLAND
#include <epoxy/egl.h>
#include <gdk/wayland/gdkwayland.h>
#endif
#ifdef GDK_WINDOWING_WIN32
#include <epoxy/wgl.h>
#include <gdk/gdkwin32.h>
#endif

namespace {
auto get_proc_address(void* /* fn_ctx */, const char* name) -> void*
{
    return reinterpret_cast<void*>(glfwGetProcAddress(name));
}
} // namespace

namespace multimedia {
MpvWrapper::MpvWrapper(std::shared_ptr<kocoro::SynchronousExecutor> executor)
    : signalShouldRender{executor->makeVolatileSignal<bool>()}
    , signalTimePos{executor->makeVolatileSignal<double>()}
    , signalEvent{executor->makeVolatileSignal<bool>()}
{
    executor->startCoro(handleEventTask());
    mpv = decltype(mpv)(mpv_create(), mpv_deleter);
    // mpv_set_option_string(mpv.get(), "terminal", "yes");
    // mpv_set_option_string(mpv.get(), "msg-level", "all=v");
    mpv_set_option_string(mpv.get(), "sid", "2");
    // mpv_set_option_string(mpv.get(), "sid", "no");
    mpv_set_option_string(mpv.get(), "audio-display", "no");
    if (mpv_initialize(mpv.get()) < 0) {
        throw std::runtime_error("could not initialize mpv context");
    }

    mpv_set_wakeup_callback(
            mpv.get(),
            [](void* self) { std::bit_cast<decltype(this), void*>(self)->signalEvent->set(true); },
            this);
    mpv_observe_property(mpv.get(), 0, "duration", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv.get(), 0, "time-pos", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv.get(), 0, "pause", MPV_FORMAT_FLAG);
    mpv_observe_property(mpv.get(), 0, "end-file", MPV_FORMAT_FLAG);
}

auto MpvWrapper::handleEventTask() -> kocoro::Task<>
{
    while (true) {
        auto _ = co_await *signalEvent;

        while (true) {
            mpv_event* event = mpv_wait_event(mpv.get(), 0);
            if (event->event_id == MPV_EVENT_NONE) {
                break;
            }
            handle_mpv_event(event);
        }
        if (!paused && timePos >= stopAtPosition) {
            pause();
        }
    }
    co_return;
}

void MpvWrapper::handle_mpv_event(mpv_event* event)
{
    switch (event->event_id) {
    case MPV_EVENT_END_FILE: {
        auto* prop = reinterpret_cast<mpv_event_end_file*>(event->data);
        if (prop->reason == mpv_end_file_reason::MPV_END_FILE_REASON_EOF) {
            stopped = true;
            timePos = duration;
            signalTimePos->set(timePos);
        }

        break;
    }
    case MPV_EVENT_PROPERTY_CHANGE: {
        auto* prop = reinterpret_cast<mpv_event_property*>(event->data);
        if (std::string{prop->name} == "time-pos") {
            if (prop->format == MPV_FORMAT_DOUBLE) {
                timePos = *reinterpret_cast<double*>(prop->data);
                signalTimePos->set(timePos);
                seeking = false;
                if (secondarySeekPosition) {
                    auto newPos = *secondarySeekPosition;
                    secondarySeekPosition.reset();
                    seek(newPos);
                }
            }
        } else if (std::string{prop->name} == "duration") {
            if (prop->format == MPV_FORMAT_DOUBLE) {
                duration = *reinterpret_cast<double*>(prop->data);
                spdlog::info("duration: {}", duration);
            }
        } else if (std::string{prop->name} == "pause") {
            if (prop->format == MPV_FORMAT_FLAG) {
                paused = static_cast<bool>(*reinterpret_cast<int*>(prop->data));
                // spdlog::info("paused: {}", paused);
            }
        }
        break;
    }
    default:
        break;
    }
}

void MpvWrapper::openFile(const std::filesystem::path& mediaFile_in)
{
    duration = 0.;
    timePos = 0.;
    if (mediaFile == mediaFile_in.string() && !stopped) {
        return;
    }
    mediaFile = mediaFile_in.string();
    if (mediaFile.empty()) {
        pause();
        closeFile();
        return;
    }
    stopped = false;
    const char* cmd[] = {"loadfile", mediaFile.c_str(), nullptr};
    mpv_command(mpv.get(), static_cast<const char**>(cmd));
    spdlog::info("mpv opening file {}", mediaFile.c_str());
    pause();
}

auto MpvWrapper::getMediaFile() const -> std::filesystem::path
{
    return mediaFile;
}

void MpvWrapper::closeFile()
{
    duration = 0.;
    timePos = 0.;
}

void MpvWrapper::play(double until)
{
    if (until == 0) {
        until = duration - 0.1F;
    }
    if (stopped) {
        openFile(mediaFile);
    }
    stopAtPosition = until;
    mpv_flag_paused = 0;
    mpv_set_property(mpv.get(), "pause", MPV_FORMAT_FLAG, &mpv_flag_paused);
    mpv_set_property_async(mpv.get(), 0, "ao-volume", MPV_FORMAT_DOUBLE, &volume);
}

void MpvWrapper::playFrom(double start)
{
    seek(start);
    play();
}

void MpvWrapper::playFragment(double start, double end)
{
    seek(start);
    play(end);
}

void MpvWrapper::pause()
{
    mpv_flag_paused = 1;
    mpv_set_property(mpv.get(), "pause", MPV_FORMAT_FLAG, &mpv_flag_paused);
}

void MpvWrapper::seek(double pos)
{
    if (seeking) {
        secondarySeekPosition = pos;
        return;
    }
    seeking = true;
    static std::string seek_position;
    seek_position = std::to_string(pos);
    const char* cmd[] = {"seek", seek_position.c_str(), "absolute", nullptr};
    mpv_command(mpv.get(), static_cast<const char**>(cmd));
}

void MpvWrapper::setSubtitle(bool enabled)
{
    static std::string enabled_str;
    enabled_str = enabled ? "yes" : "no";
    const char* cmd[] = {"set", "sub-visibility", enabled_str.c_str(), nullptr};
    mpv_command(mpv.get(), static_cast<const char**>(cmd));
}

auto MpvWrapper::getDuration() const -> double
{
    return duration;
}

auto MpvWrapper::getTimePos() const -> double
{
    return timePos;
}

void MpvWrapper::initGL()
{
    mpv_set_option_string(mpv.get(), "vo", "libmpv");

    mpv_opengl_init_params gl_init_params{get_proc_address, nullptr};
    static std::string renderType = MPV_RENDER_API_TYPE_OPENGL;
    int adv = 1;
    int block = 0;
    mpv_render_param params[]{
            {MPV_RENDER_PARAM_API_TYPE, renderType.data()},
            {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &gl_init_params},
            {MPV_RENDER_PARAM_ADVANCED_CONTROL, &adv},
            {MPV_RENDER_PARAM_BLOCK_FOR_TARGET_TIME, &block},
            {MPV_RENDER_PARAM_INVALID, nullptr}};

    mpv_render_context* mpv_gl_temp{};
    if (mpv_render_context_create(&mpv_gl_temp, mpv.get(), static_cast<mpv_render_param*>(params)) < 0) {
        throw std::runtime_error("failed to initialize mpv GL context");
    }
    renderCtx = decltype(renderCtx)(mpv_gl_temp, renderCtx_deleter);
    mpv_render_context_set_update_callback(
            renderCtx.get(),
            &onMpvUpdate,
            this);
}

auto MpvWrapper::render(GLuint fbo, int width, int height) -> int64_t
{
    auto nextFrameTargetTime = getNextFrameTargetTime();

    int flip_y{0};
    mpv_opengl_fbo mpfbo{static_cast<int>(fbo), width, height, 0};
    int adv = 1;
    int block = 0;
    mpv_render_param params[]{
            {MPV_RENDER_PARAM_OPENGL_FBO, &mpfbo},
            {MPV_RENDER_PARAM_FLIP_Y, &flip_y},
            {MPV_RENDER_PARAM_ADVANCED_CONTROL, &adv},
            {MPV_RENDER_PARAM_BLOCK_FOR_TARGET_TIME, &block},
            {MPV_RENDER_PARAM_INVALID, nullptr},
    };
    mpv_render_context_render(renderCtx.get(), static_cast<mpv_render_param*>(params));
    return nextFrameTargetTime;
}

auto MpvWrapper::getTime() const -> int64_t
{
    return mpv_get_time_ns(mpv.get());
}

auto MpvWrapper::SignalShouldRender() const -> kocoro::VolatileSignal<bool>&
{
    return *signalShouldRender;
}

auto MpvWrapper::SignalTimePos() const -> kocoro::VolatileSignal<double>&
{
    return *signalTimePos;
}

auto MpvWrapper::getNextFrameTargetTime() const -> int64_t
{
    mpv_render_frame_info frameInfo;
    mpv_render_param param{
            MPV_RENDER_PARAM_NEXT_FRAME_INFO,
            &frameInfo,
    };
    mpv_render_context_get_info(renderCtx.get(), param);
    return frameInfo.target_time;
}

void MpvWrapper::onMpvUpdate(void* _mpv)
{
    auto* mpv = static_cast<MpvWrapper*>(_mpv);
    if (!mpv->renderCtx) {
        return;
    }
    if (0 != (mpv_render_context_update(mpv->renderCtx.get()) & MPV_RENDER_UPDATE_FRAME)) {
        mpv->signalShouldRender->set(true);
    }
}

} // namespace multimedia
