#pragma once

#include <gtkmm.h>
#include <multimedia/MediaPlayer.h>
#include <utils/Property.h>

enum class media {
    start,
    stop,
    pause
};

class MediaButton : public Gtk::Box
{
public:
    MediaButton(std::shared_ptr<MediaPlayer>, Gtk::Orientation orientation);
    MediaButton(std::shared_ptr<MediaPlayer>);
    ~MediaButton() override = default;
    utl::Property<media> action;

    void signal_start_connect(std::function<void(std::shared_ptr<MediaPlayer>)>);
    void signal_pause_connect(std::function<void(std::shared_ptr<MediaPlayer>)>);
    void signal_stop_connect(std::function<void(std::shared_ptr<MediaPlayer>)>);

protected:
    void onBtnClick();
    virtual void onBtnClick_post(){};
    std::shared_ptr<MediaPlayer> mediaPlayer;
    media nextAction = media::start;
    Gtk::Button button;
    utl::ObserverCollection observers;

    std::function<void(std::shared_ptr<MediaPlayer>)> signal_start;
    std::function<void(std::shared_ptr<MediaPlayer>)> signal_pause;
    std::function<void(std::shared_ptr<MediaPlayer>)> signal_stop;
};

class PlayStopButton : public MediaButton
{
public:
    PlayStopButton(std::shared_ptr<MediaPlayer>);
    void signal_pause_connect(std::function<void(std::shared_ptr<MediaPlayer>)>) = delete;

private:
    void onBtnClick_post();
    std::shared_ptr<utl::ObserverBase> observer_stopped;
};

class PlayPauseButton : public MediaButton
{
public:
    PlayPauseButton(std::shared_ptr<MediaPlayer>);
    void signal_stop_connect(std::function<void(std::shared_ptr<MediaPlayer>)>) = delete;

private:
    void onBtnClick_post();
    std::shared_ptr<utl::ObserverBase> observer_paused;
};

class BtnGrpForwardBackward : public Gtk::Box
{
public:
    BtnGrpForwardBackward();
    void setSensitive(bool sensitive = true);
    void setIndividualSensitivity(bool sensitive_skipBackwardBtn,
                                  bool sensitive_seekBackwardBtn,
                                  bool sensitive_seekForwardBtn,
                                  bool sensitive_skipForwardBtn);
    void skipBackwardClick(const std::function<void()>&);
    void seekBackwardClick(const std::function<void()>&);
    void seekForwardClick(const std::function<void()>&);
    void skipForwardClick(const std::function<void()>&);

private:
    Gtk::Button skipBackwardBtn;
    Gtk::Button seekBackwardBtn;
    Gtk::Button seekForwardBtn;
    Gtk::Button skipForwardBtn;
};
