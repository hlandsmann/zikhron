#include <DataThread.h>
#include <VideoSpace.h>
#include <spdlog/spdlog.h>
#include <filesystem>
#ifdef GDK_WINDOWING_X11
#include <epoxy/glx.h>
// #include <gdk/x11/gdkx.h>
#endif
#ifdef GDK_WINDOWING_WAYLAND
#include <epoxy/egl.h>
// #include <gdk/wayland/gdkwayland.h>
#endif
#ifdef GDK_WINDOWING_WIN32
#include <epoxy/wgl.h>
#include <gdk/gdkwin32.h>
#endif

namespace fs = std::filesystem;

VideoSpace::VideoSpace() {
    set_orientation(Gtk::Orientation::VERTICAL);
    set_vexpand();
    set_spacing(16);
    createGlArea();
    createControlButtons();
}

void VideoSpace::createGlArea() {
    glArea->set_expand(true);
    glArea->set_size_request(1920, 1080);
    glArea->set_auto_render(true);
    glArea->signal_render().connect(sigc::mem_fun(*this, &VideoSpace::render), false);
    glArea->signal_realize().connect(sigc::mem_fun(*this, &VideoSpace::realize));
    // m_GLArea.signal_unrealize().connect(sigc::mem_fun(*this, &MyWindow::unrealize));

    append(*glArea);
}

void VideoSpace::createControlButtons() {
    controlBtnBox.set_orientation(Gtk::Orientation::HORIZONTAL);
    btnOpenFile.set_label("Open Video");
    btnOpenFile.signal_clicked().connect([this]() { createFileChooserDialog(); });
    controlBtnBox.append(btnOpenFile);
    controlBtnBox.append(separator1);
    separator1.set_expand();

    setPlayPauseBtnIcon();
    btnPlayPause.signal_clicked().connect([this]() {
        mediaPlayer->play(mediaPlayer->is_paused());
        setPlayPauseBtnIcon();
    });

    controlBtnBox.append(btnPlayPause);
    // separator2.set_expand();
    // controlBtnBox.append(separator2);
    progressBar.set_expand();
    controlBtnBox.append(progressBar);
    append(controlBtnBox);
}

void VideoSpace::setPlayPauseBtnIcon() {
    if (mediaPlayer->is_paused())
        btnPlayPause.set_image_from_icon_name("media-playback-start");
    else
        btnPlayPause.set_image_from_icon_name("media-playback-pause");
}

void VideoSpace::createFileChooserDialog() {
    auto dialog = new Gtk::FileChooserDialog("Please choose a file", Gtk::FileChooser::Action::OPEN);
    Gtk::Window *parent = dynamic_cast<Gtk::Window *>(this->get_root());
    dialog->set_transient_for(*parent);

    if (auto videoFile = DataThread::get().zikhronCfg.cfgMain.lastVideoFile;
        not videoFile.empty() && fs::exists(videoFile)) {
        videoFile.parent_path();
        dialog->set_current_folder(Gio::File::create_for_path(videoFile.parent_path().string()));
    }
    dialog->set_modal(true);
    dialog->signal_response().connect(
        sigc::bind(sigc::mem_fun(*this, &VideoSpace::on_file_dialog_response), dialog));

    // Add response buttons to the dialog:
    dialog->add_button("_Cancel", Gtk::ResponseType::CANCEL);
    dialog->add_button("_Open", Gtk::ResponseType::OK);

    // Add filters, so that only certain file types can be selected:
    auto filter_video = Gtk::FileFilter::create();
    filter_video->set_name("Video, *.mkv");
    filter_video->add_mime_type("video/x-matroska");
    dialog->add_filter(filter_video);

    auto filter_any = Gtk::FileFilter::create();
    filter_any->set_name("Any files");
    filter_any->add_pattern("*");
    dialog->add_filter(filter_any);

    // Show the dialog and wait for a user response:
    dialog->show();
}

void VideoSpace::on_file_dialog_response(int response_id, Gtk::FileChooserDialog *dialog) {
    switch (response_id) {
    case Gtk::ResponseType::OK: {
        auto filename = dialog->get_file()->get_path();
        spdlog::info("File selected: {}", filename);
        DataThread::get().zikhronCfg.cfgMain.lastVideoFile = filename;
        mediaPlayer->openFile(filename);
        break;
    }
    default: {
        spdlog::info("No file selected.");
        break;
    }
    }
    delete dialog;
}

void VideoSpace::switchPage(bool active_in) {
    active = active_in;
    spdlog::info("VideoSpace active: {}", active);
}

bool VideoSpace::render(const Glib::RefPtr<Gdk::GLContext> &context) {
    bool result = false;
    if (mediaPlayer)
        result = mediaPlayer->render(context);
    if (result)
        glFlush();
    return result;
}

void VideoSpace::realize() {
    mediaPlayer->initGL(glArea);
    // auto cap_mediaPlayer = mediaPlayer;
    DataThread::get().dispatch_arbitrary([mediaPlayer = mediaPlayer]() {
        auto filename = DataThread::get().zikhronCfg.cfgMain.lastVideoFile;
        if (not filename.empty() && std::filesystem::exists(filename))
            mediaPlayer->openFile(filename);
    });
}