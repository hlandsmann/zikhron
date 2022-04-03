#include <DataThread.h>
#include <VideoSpace.h>
#include <spdlog/spdlog.h>
#include <filesystem>
namespace fs = std::filesystem;

VideoSpace::VideoSpace() {
    set_orientation(Gtk::Orientation::VERTICAL);
    set_vexpand();
    set_spacing(16);
    createGlArea();
    createControlButtons();
}

void VideoSpace::createGlArea() {
    glArea.set_expand(true);
    glArea.set_size_request(1920, 1080);
    glArea.set_auto_render(true);
    glArea.signal_render().connect(sigc::mem_fun(*this, &VideoSpace::render), false);
    glArea.signal_realize().connect(sigc::mem_fun(*this, &VideoSpace::realize));
    // m_GLArea.signal_unrealize().connect(sigc::mem_fun(*this, &MyWindow::unrealize));

    append(glArea);
}

void VideoSpace::createControlButtons() {
    controlBtnBox.set_orientation(Gtk::Orientation::HORIZONTAL);
    btnOpenFile.set_label("Open Video");
    btnOpenFile.signal_clicked().connect([this]() { createFileChooserDialog(); });
    controlBtnBox.append(btnOpenFile);
    controlBtnBox.append(separator1);
    separator1.set_expand();

    // icon_pause.set_from_icon_name("media-playback-start");
    btnPlayPause.set_image_from_icon_name("media-playback-start");
    btnPlayPause.set_image_from_icon_name("media-playback-pause");
    controlBtnBox.append(btnPlayPause);
    separator2.set_expand();
    controlBtnBox.append(separator2);

    append(controlBtnBox);
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
        break;
    }
    default: {
        spdlog::info("No file selected.");
        break;
    }
    }
    delete dialog;
}
void VideoSpace::switchPage(bool active) { spdlog::warn("Switch: {}", active); }

bool VideoSpace::render(const Glib::RefPtr<Gdk::GLContext> &context) { return true; }
void VideoSpace::realize() {}