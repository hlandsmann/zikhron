#pragma once

#include <MediaButtons.h>
#include <NotebookPage.h>
#include <SubtitleComboBox.h>
#include <SubtitleOverlay.h>
#include <gtkmm.h>
#include <multimedia/MediaPlayer.h>
#include <multimedia/Subtitles.h>
#include <utils/Property.h>

#include <list>

class VideoSpace : public Gtk::Box
    , public NotebookPage
{
public:
    VideoSpace(Gtk::Overlay&);
    void switchPage(bool);

private:
    void createGlArea();
    void createControlButtons();
    void createSubtitleOverlay();
    void createFileChooserDialog();
    void on_file_dialog_response(int response_id, Gtk::FileChooserDialog* dialog);
    auto signal_render(const Glib::RefPtr<Gdk::GLContext>& context) -> bool;
    void signal_realize();
    void signal_resize(int width, int height);
    void subtitleLoading_callback(bool finished);
    std::shared_ptr<MediaPlayer> mediaPlayer;

    Gtk::Overlay& overlay;
    utl::ObserverCollection observers;
    Gtk::Box videoBox;
    Gtk::Box controlBtnBox;
    SubtitleComboBox subtitleComboBox;
    Gtk::Button btnOpenFile;
    PlayPauseButton btnPlayPause{mediaPlayer};
    Gtk::Separator separator1, separator2;
    std::shared_ptr<Gtk::GLArea> glArea = std::make_shared<Gtk::GLArea>();
    Gtk::ProgressBar progressBar;
    std::shared_ptr<SubtitleDecoder> subtitleDecoder;
    std::unique_ptr<SubtitleOverlay> subtitleOverlay;

    utl::Property<std::string> filename;
    // std::shared_ptr<utl::ObserverBase> progressObserver;

    utl::Property<bool> active = false;
};
