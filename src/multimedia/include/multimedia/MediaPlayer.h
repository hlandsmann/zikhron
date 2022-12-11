#pragma once
#include <gtkmm.h>
#include <mpv/client.h>
#include <mpv/render_gl.h>
#include <utils/Property.h>
#include <filesystem>
#include <functional>
#include <memory>

class MediaPlayer {
public:
    MediaPlayer();
    void openFile(const std::filesystem::path& mediaFile);
    void play(bool play = true);
    void play_fragment(double start, double end);
    void pause(bool pause = true);
    void seek(double pos);
    auto is_paused() const -> bool { return paused; }
    void initGL(const std::shared_ptr<Gtk::GLArea>& glArea);
    auto render(const Glib::RefPtr<Gdk::GLContext>& context) -> bool;

    auto get_duration() const -> const utl::Property<double>& { return duration; }

private:
    void observe_duration(const std::function<void(double)> observer);
    void observe_timePos(const std::function<void(double)> observer);
    void handle_mpv_event(mpv_event* event);
    void on_mpv_events();

    Glib::Dispatcher dispatch_mpvEvent;
    Glib::Dispatcher dispatch_render;

    std::function<void(mpv_handle*)> mpv_deleter = [](mpv_handle* mpvHandle) {
        mpv_terminate_destroy(mpvHandle);
    };
    std::unique_ptr<mpv_handle, decltype(mpv_deleter)> mpv;

    std::function<void(mpv_render_context*)> renderCtx_deleter = [](mpv_render_context* render_ctx) {
        mpv_render_context_free(render_ctx);
    };
    std::unique_ptr<mpv_render_context, decltype(renderCtx_deleter)> mpv_gl;
    std::shared_ptr<Gtk::GLArea> glArea;
    std::string mediaFile;

    utl::ObserverCollection observers;
    utl::Property<double> duration;
    utl::Property<double> timePos;
    int paused = true;

    double stopAtPosition = 0;
};