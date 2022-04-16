#pragma once

#include <NotebookPage.h>
#include <gtkmm.h>
#include <multimedia/Mediaplayer.h>


class VideoSpace : public Gtk::Box, public NotebookPage {
public:
    VideoSpace();
    void switchPage(bool);

private:
    void createGlArea();
    void createControlButtons();
    void setPlayPauseBtnIcon();
    void createFileChooserDialog();
    void on_file_dialog_response(int response_id, Gtk::FileChooserDialog* dialog);
    bool render(const Glib::RefPtr<Gdk::GLContext>& context);
    void realize();
    Gtk::Box controlBtnBox;
    Gtk::Button btnOpenFile;
    Gtk::Button btnPlayPause;
    Gtk::Separator separator1, separator2;
    std::shared_ptr<Gtk::GLArea> glArea = std::make_shared<Gtk::GLArea>();
    Gtk::ProgressBar progressBar;
    std::shared_ptr<MediaPlayer> mediaPlayer = std::make_shared<MediaPlayer>();

    bool active = false;
};