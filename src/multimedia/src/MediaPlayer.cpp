#include <MediaPlayer.h>
#include <spdlog/spdlog.h>
#include <bit>

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
static void *get_proc_address([[maybe_unused]] void *fn_ctx, const gchar *name) {
    GdkDisplay *display = gdk_display_get_default();

#ifdef GDK_WINDOWING_WAYLAND
    if (GDK_IS_WAYLAND_DISPLAY(display))
        return reinterpret_cast<void *>(eglGetProcAddress(name));
#endif
#ifdef GDK_WINDOWING_X11
    if (GDK_IS_X11_DISPLAY(display))
        return (void *)(intptr_t)glXGetProcAddressARB((const GLubyte *)name);
#endif
#ifdef GDK_WINDOWING_WIN32
    if (GDK_IS_WIN32_DISPLAY(display))
        return wglGetProcAddress(name);
#endif
    g_assert_not_reached();
    return NULL;
}
}  // namespace

MediaPlayer::MediaPlayer() {
    dispatch_render.connect([this]() {
        if (glArea)
            glArea->queue_render();
    });
    dispatch_mpvEvent.connect([this]() { on_mpv_events(); });

    mpv = decltype(mpv)(mpv_create(), mpv_deleter);
    mpv_set_option_string(mpv.get(), "terminal", "yes");
    // mpv_set_option_string(mpv.get(), "msg-level", "all=v");
    mpv_set_option_string(mpv.get(), "sid", "no");
    mpv_set_option_string(mpv.get(), "audio-display", "no");
    if (mpv_initialize(mpv.get()) < 0)
        throw std::runtime_error("could not initialize mpv context");

    mpv_set_wakeup_callback(
        mpv.get(),
        [](void *self) { std::bit_cast<decltype(this), void *>(self)->dispatch_mpvEvent.emit(); },
        this);
    mpv_observe_property(mpv.get(), 0, "duration", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv.get(), 0, "time-pos", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv.get(), 0, "pause", MPV_FORMAT_FLAG);

    pause();

    observe_timePos([this](double time_pos) {
        if (time_pos >= stopAtPosition) {
            stopAtPosition = duration;
            pause();
        }
    });
}

void MediaPlayer::openFile(const std::filesystem::path &mediaFile_in) {
    duration = 0.;
    timePos = 0.;
    mediaFile = mediaFile_in.string();
    const char *cmd[] = {"loadfile", mediaFile.c_str(), NULL};
    mpv_command(mpv.get(), cmd);
    spdlog::info("mpv opening file {}", mediaFile.c_str());
    pause();
}

void MediaPlayer::play_fragment(double start, double end) {
    seek(start);
    play(end);
}

void MediaPlayer::play(double until) {
    if (until == 0)
        until = duration;
    stopAtPosition = until;
    mpv_flag_paused = 0;
    mpv_set_property(mpv.get(), "pause", MPV_FORMAT_FLAG, &mpv_flag_paused);
}

void MediaPlayer::pause() {
    mpv_flag_paused = 1;
    mpv_set_property(mpv.get(), "pause", MPV_FORMAT_FLAG, &mpv_flag_paused);
}

void MediaPlayer::seek(double pos) {
    static std::string seek_position;
    seek_position = std::to_string(pos);
    const char *cmd[] = {"seek", seek_position.c_str(), "absolute", NULL};
    mpv_command(mpv.get(), cmd);
}

void MediaPlayer::initGL(const std::shared_ptr<Gtk::GLArea> &glArea_in) {
    mpv_opengl_init_params gl_init_params{get_proc_address, nullptr, nullptr};
    mpv_render_param params[]{
        {MPV_RENDER_PARAM_API_TYPE, const_cast<char *>(MPV_RENDER_API_TYPE_OPENGL)},
        {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &gl_init_params},
        {MPV_RENDER_PARAM_INVALID, nullptr}};

    mpv_render_context *mpv_gl_temp;
    if (mpv_render_context_create(&mpv_gl_temp, mpv.get(), params) < 0)
        throw std::runtime_error("failed to initialize mpv GL context");
    mpv_gl = decltype(mpv_gl)(mpv_gl_temp, renderCtx_deleter);
    mpv_render_context_set_update_callback(
        mpv_gl.get(),
        [](void *ctx) {
            auto &self = *std::bit_cast<MediaPlayer *, void *>(ctx);
            self.dispatch_render.emit();
        },
        std::bit_cast<void *, MediaPlayer *>(this));
    glArea = glArea_in;
}

bool MediaPlayer::render([[maybe_unused]] const Glib::RefPtr<Gdk::GLContext> &context) {
    if (mpv_gl == nullptr || glArea == nullptr)
        return false;

    gint fbo = -1;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fbo);

    int width = glArea->get_size(Gtk::Orientation::HORIZONTAL);
    int height = glArea->get_size(Gtk::Orientation::VERTICAL);
    mpv_opengl_fbo mpfbo{fbo, width, height, 0};
    int flip_y{1};

    mpv_render_param params[] = {{MPV_RENDER_PARAM_OPENGL_FBO, &mpfbo},
                                 {MPV_RENDER_PARAM_FLIP_Y, &flip_y},
                                 {MPV_RENDER_PARAM_INVALID, nullptr}};
    mpv_render_context_render(mpv_gl.get(), params);
    // glViewport(0,0,width,height);
    return true;
}

void MediaPlayer::on_mpv_events() {
    // Process all events, until the event queue is empty.
    while (mpv) {
        mpv_event *event = mpv_wait_event(mpv.get(), 0);
        if (event->event_id == MPV_EVENT_NONE) {
            break;
        }
        handle_mpv_event(event);
    }
}

void MediaPlayer::handle_mpv_event(mpv_event *event) {
    switch (event->event_id) {
    case MPV_EVENT_PROPERTY_CHANGE: {
        mpv_event_property *prop = (mpv_event_property *)event->data;
        if (strcmp(prop->name, "time-pos") == 0) {
            if (prop->format == MPV_FORMAT_DOUBLE) {
                timePos = *(double *)prop->data;
                if (duration - timePos <= 1) {
                    enable_stop_timer();
                }
            }
        } else if (strcmp(prop->name, "duration") == 0) {
            if (prop->format == MPV_FORMAT_DOUBLE) {
                duration = *(double *)prop->data;
                spdlog::info("duration: {}", duration);
            }
        } else if (strcmp(prop->name, "pause") == 0) {
            if (prop->format == MPV_FORMAT_FLAG) {
                paused = *(int *)prop->data;
                // spdlog::info("paused: {}", paused);
            }
        }
        break;
    }
    default: break;
    }
}

void MediaPlayer::observe_duration(const std::function<void(double)> observer) {
    auto durationObserver = duration.observe(observer);
    observers.push(durationObserver);
}

void MediaPlayer::observe_timePos(const std::function<void(double)> observer) {
    auto timePosObserver = timePos.observe(observer);
    observers.push(timePosObserver);
}

void MediaPlayer::enable_stop_timer() {
    if (!timer_running) {
        sigc::slot<bool()> slot = sigc::bind([this](int _timer_id) { return timer_stop(_timer_id); },
                                             timer_id);
        timer_connection = Glib::signal_timeout().connect(slot, 1000);
    }
    timer_running = true;
}

void MediaPlayer::disable_stop_timer() {
    spdlog::info("Stop emergency disabled!");
    timer_connection.disconnect();
    timer_running = false;
}

bool MediaPlayer::timer_stop(int /*timer_id*/) {
    if (paused) {
        disable_stop_timer();
        return false;
    }
    spdlog::warn("Mediaplayer - Timer file reload!");
    pause();
    openFile(mediaFile);
    return true;
}