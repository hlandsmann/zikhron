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
#include <filesystem>
#include <functional>
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
auto get_proc_address(void* fn_ctx, const char* name) -> void*
{
    return reinterpret_cast<void*>(glfwGetProcAddress(name));
}
} // namespace

MpvWrapper::MpvWrapper()
{
    // dispatch_render.connect([this]() {
    //     if (glArea)
    //         glArea->queue_render();
    // });
    // dispatch_mpvEvent.connect([this]() { on_mpv_events(); });

    mpv = decltype(mpv)(mpv_create(), mpv_deleter);
    mpv_set_option_string(mpv.get(), "terminal", "yes");
    // mpv_set_option_string(mpv.get(), "msg-level", "all=v");
    mpv_set_option_string(mpv.get(), "sid", "no");
    mpv_set_option_string(mpv.get(), "audio-display", "no");
    if (mpv_initialize(mpv.get()) < 0) {
        throw std::runtime_error("could not initialize mpv context");
    }

    // mpv_set_wakeup_callback(
    //         mpv.get(),
    //         [](void* self) { std::bit_cast<decltype(this), void*>(self)->dispatch_mpvEvent.emit(); },
    //         this);
    mpv_observe_property(mpv.get(), 0, "duration", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv.get(), 0, "time-pos", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv.get(), 0, "pause", MPV_FORMAT_FLAG);

    pause();

    // observe_timePos([this](double time_pos) {
    //     if (time_pos >= stopAtPosition) {
    //         stopAtPosition = duration;
    //         pause();
    //     }
    // });
}

void MpvWrapper::openFile(const std::filesystem::path& mediaFile_in)
{
    duration = 0.;
    timePos = 0.;
    mediaFile = mediaFile_in.string();
    if (mediaFile.empty()) {
        pause();
        closeFile();
        return;
    }
    const char* cmd[] = {"loadfile", mediaFile.c_str(), nullptr};
    mpv_command(mpv.get(), static_cast<const char**>(cmd));
    spdlog::info("mpv opening file {}", mediaFile.c_str());
    pause();
}

void MpvWrapper::closeFile()
{
    duration = 0.;
    timePos = 0.;
}

void MpvWrapper::play_fragment(double start, double end)
{
    seek(start);
    play(end);
}

void MpvWrapper::play(double until)
{
    if (until == 0) {
        until = duration;
    }
    stopAtPosition = until;
    mpv_flag_paused = 0;
    mpv_set_property(mpv.get(), "pause", MPV_FORMAT_FLAG, &mpv_flag_paused);
    mpv_set_property_async(mpv.get(), 0, "ao-volume", MPV_FORMAT_DOUBLE, &volume);
}

void MpvWrapper::pause()
{
    mpv_flag_paused = 1;
    mpv_set_property(mpv.get(), "pause", MPV_FORMAT_FLAG, &mpv_flag_paused);
}

void MpvWrapper::seek(double pos)
{
    static std::string seek_position;
    seek_position = std::to_string(pos);
    const char* cmd[] = {"seek", seek_position.c_str(), "absolute", nullptr};
    mpv_command(mpv.get(), static_cast<const char**>(cmd));
}

void MpvWrapper::initGL(/* const std::shared_ptr<Gtk::GLArea>& glArea_in */)
{
    mpv_opengl_init_params gl_init_params{get_proc_address, nullptr};
    static std::string renderType = MPV_RENDER_API_TYPE_OPENGL;
    mpv_render_param params[]{
            {MPV_RENDER_PARAM_API_TYPE, renderType.data()},
            {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &gl_init_params},
            {MPV_RENDER_PARAM_INVALID, nullptr}};

    mpv_render_context* mpv_gl_temp{};
    if (mpv_render_context_create(&mpv_gl_temp, mpv.get(), static_cast<mpv_render_param*>(params)) < 0) {
        throw std::runtime_error("failed to initialize mpv GL context");
    }
    mpv_gl = decltype(mpv_gl)(mpv_gl_temp, renderCtx_deleter);
}

auto MpvWrapper::render(int width, int height) -> bool
{
    if (mpv_gl == nullptr /* || glArea == nullptr */) {
        return false;
    }

    int fbo = -1;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fbo);

    mpv_opengl_fbo mpfbo{0, width, height, 0};
    int flip_y{1};

    mpv_render_param params[] = {{MPV_RENDER_PARAM_OPENGL_FBO, &mpfbo},
                                 {MPV_RENDER_PARAM_FLIP_Y, &flip_y},
                                 {MPV_RENDER_PARAM_INVALID, nullptr}};
    mpv_render_context_render(mpv_gl.get(), static_cast<mpv_render_param*>(params));
    return true;
}

void MpvWrapper::on_mpv_events()
{
    // Process all events, until the event queue is empty.
    while (mpv) {
        mpv_event* event = mpv_wait_event(mpv.get(), 0);
        if (event->event_id == MPV_EVENT_NONE) {
            break;
        }
        handle_mpv_event(event);
    }
}

void MpvWrapper::handle_mpv_event(mpv_event* event)
{
    switch (event->event_id) {
    case MPV_EVENT_PROPERTY_CHANGE: {
        auto* prop = reinterpret_cast<mpv_event_property*>(event->data);
        if (std::string{prop->name} == "time-pos") {
            if (prop->format == MPV_FORMAT_DOUBLE) {
                timePos = *reinterpret_cast<double*>(prop->data);
                if (duration - timePos <= 1) {
                    enable_stop_timer();
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
                spdlog::info("paused: {}", paused);
            }
        }
        break;
    }
    default:
        break;
    }
}

// void MpvWrapper::observe_duration(const std::function<void(double)> observer)
// {
//     auto durationObserver = duration.observe(observer);
//     observers.push(durationObserver);
// }

// void MpvWrapper::observe_timePos(const std::function<void(double)> observer)
// {
//     auto timePosObserver = timePos.observe(observer);
//     observers.push(timePosObserver);
// }

void MpvWrapper::enable_stop_timer()
{
    // if (!timer_running) {
    //     sigc::slot<bool()> slot = sigc::bind([this](int _timer_id) { return timer_stop(_timer_id); },
    //                                          timer_id);
    //     timer_connection = Glib::signal_timeout().connect(slot, 1000);
    // }
    timer_running = true;
}

void MpvWrapper::disable_stop_timer()
{
    spdlog::info("Stop emergency disabled!");
    // timer_connection.disconnect();
    timer_running = false;
}

auto MpvWrapper::timer_stop(int /*timer_id*/) -> bool
{
    if (paused) {
        disable_stop_timer();
        return false;
    }
    spdlog::warn("Mediaplayer - Timer file reload!");
    pause();
    openFile(mediaFile);
    return true;
}
