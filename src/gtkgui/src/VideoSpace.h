#pragma once

#include <NotebookPage.h>
#include <gtkmm.h>
#include <multimedia/mediaplayer.h>

class VideoSpace : public Gtk::Box, public NotebookPage {
public:
    VideoSpace();
    void switchPage(bool);

private:
    void createGlArea();
    void createControlButtons();
    void createFileChooserDialog();
    void on_file_dialog_response(int response_id, Gtk::FileChooserDialog* dialog);
    bool render(const Glib::RefPtr<Gdk::GLContext>& context);
    void realize();
    Gtk::Box controlBtnBox;
    Gtk::Button btnOpenFile;
    Gtk::Button btnPlayPause;
    Gtk::Separator separator1, separator2;
    Gtk::GLArea glArea;
};