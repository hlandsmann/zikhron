#pragma once

#include <NotebookPage.h>
#include <SubtitleOverlay.h>
#include <gtkmm.h>
#include <multimedia/Mediaplayer.h>
#include <multimedia/Subtitles.h>
#include <utils/Property.h>
#include <list>

class VideoSpace : public Gtk::Box, public NotebookPage {
public:
    VideoSpace(Gtk::Overlay&);
    void switchPage(bool);

private:
    void createGlArea();
    void createControlButtons();
    void createSubtitleOverlay();
    void setPlayPauseBtnIcon();
    void createFileChooserDialog();
    void on_file_dialog_response(int response_id, Gtk::FileChooserDialog* dialog);
    bool signal_render(const Glib::RefPtr<Gdk::GLContext>& context);
    void signal_realize();
    void signal_resize(int width, int height);
    void subtitleLoading_callback(bool finished);

    Gtk::Overlay& overlay;
    utl::ObserverCollection observers;
    Gtk::Box videoBox;
    Gtk::Box controlBtnBox;
    Gtk::Button btnOpenFile;
    Gtk::Button btnPlayPause;
    Gtk::Separator separator1, separator2;
    std::shared_ptr<Gtk::GLArea> glArea = std::make_shared<Gtk::GLArea>();
    Gtk::ProgressBar progressBar;
    std::shared_ptr<MediaPlayer> mediaPlayer = std::make_shared<MediaPlayer>();
    std::shared_ptr<SubtitleDecoder> subtitleDecoder;
    std::unique_ptr<SubtitleOverlay> subtitleOverlay;

    utl::Property<std::string> filename;
    // std::shared_ptr<utl::ObserverBase> progressObserver;

    utl::Property<bool> active = false;
};