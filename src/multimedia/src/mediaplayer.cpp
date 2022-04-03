#include <mediaplayer.h>
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

    mpv = decltype(mpv)(mpv_create(), mpv_deleter);
    mpv_set_option_string(mpv.get(), "terminal", "yes");
    mpv_set_option_string(mpv.get(), "msg-level", "all=v");

    if (mpv_initialize(mpv.get()) < 0)
        throw std::runtime_error("could not initialize mpv context");
}

void MediaPlayer::openFile(const std::filesystem::path &videoFile_in) {
    videoFile = videoFile_in.string();
    const char *cmd[] = {"loadfile", videoFile.c_str(), NULL};
    mpv_command(mpv.get(), cmd);
    spdlog::info("opening file {}", videoFile.c_str());
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
    return true;
}
