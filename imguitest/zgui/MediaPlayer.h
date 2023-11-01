#pragma once
#include <mpv/client.h>
#include <mpv/render.h>
#include <mpv/render_gl.h>

#include <filesystem>
#include <functional>
#include <memory>
#include <string>

class MediaPlayer
{
public:
    MediaPlayer();

    // auto property_duration() const -> const utl::Property<double>& { return duration; }
    // auto property_paused() const -> const utl::Property<bool>& { return paused; }
    // auto property_timePos() const -> const utl::Property<double>& { return timePos; }

    void openFile(const std::filesystem::path& mediaFile);
    void play(double until = 0.0);
    void play_fragment(double start, double end);
    void pause();
    void seek(double pos);
    [[nodiscard]] auto is_paused() const -> bool { return paused; }
    void initGL(/* const std::shared_ptr<Gtk::GLArea>& glArea */);
    auto render(int width, int height) -> bool;

private:
    void observe_duration(std::function<void(double)> observer);
    void observe_timePos(std::function<void(double)> observer);
    void handle_mpv_event(mpv_event* event);
    void on_mpv_events();

    void closeFile();
    void enable_stop_timer();
    void disable_stop_timer();
    auto timer_stop(int timer_id) -> bool;
    int timer_id = 0;
    bool timer_running = false;
    // sigc::connection timer_connection;
    //
    // Glib::Dispatcher dispatch_mpvEvent;
    // Glib::Dispatcher dispatch_render;

    std::function<void(mpv_handle*)> mpv_deleter = [](mpv_handle* mpvHandle) {
        mpv_terminate_destroy(mpvHandle);
    };
    std::unique_ptr<mpv_handle, decltype(mpv_deleter)> mpv;

    std::function<void(mpv_render_context*)> renderCtx_deleter = [](mpv_render_context* render_ctx) {
        mpv_render_context_free(render_ctx);
    };
    std::unique_ptr<mpv_render_context, decltype(renderCtx_deleter)> mpv_gl;
    // std::shared_ptr<Gtk::GLArea> glArea;
    std::string mediaFile;

    // utl::ObserverCollection observers;
    // utl::Property<double> duration;
    // utl::Property<double> timePos;
    int mpv_flag_paused = 1;
    // utl::Property<bool> paused = true;
    bool paused{false};
    double stopAtPosition = 0;
    double volume{100.F};
    double duration{};
    double timePos{};
    // GLuint tex{};
};
