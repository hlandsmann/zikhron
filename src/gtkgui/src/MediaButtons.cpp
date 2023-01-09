#include <MediaButtons.h>

MediaButton::MediaButton(MediaPlayer& _mediaPlayer, Gtk::Orientation orientation)
    : mediaPlayer(_mediaPlayer) {
    set_orientation(orientation);
    append(button);
    observers.push(action.observe([this](const media& _action) {
        switch (_action) {
        case media::start: button.set_image_from_icon_name("media-playback-start"); break;
        case media::stop: button.set_image_from_icon_name("media-playback-stop"); break;
        case media::pause: button.set_image_from_icon_name("media-playback-pause"); break;
        }
    }));
    button.signal_clicked().connect([this]() { onBtnClick(); });
}
MediaButton::MediaButton(MediaPlayer& _mediaPlayer)
    : MediaButton(_mediaPlayer, Gtk::Orientation::VERTICAL) {}
void MediaButton::signal_start_connect(std::function<void(MediaPlayer&)> signal_fun) {
    signal_start = signal_fun;
}

void MediaButton::signal_pause_connect(std::function<void(MediaPlayer&)> signal_fun) {
    signal_pause = signal_fun;
}

void MediaButton::signal_stop_connect(std::function<void(MediaPlayer&)> signal_fun) {
    signal_stop = signal_fun;
}

void MediaButton::onBtnClick() {
    switch (action) {
    case media::start:
        if (signal_start)
            signal_start(mediaPlayer);
        break;
    case media::stop:
        if (signal_stop)
            signal_stop(mediaPlayer);
        break;
    case media::pause:
        if (signal_pause)
            signal_pause(mediaPlayer);
        break;
    }
    onBtnClick_post();
    action = nextAction;
}

/** PlayStop *************************************************************************************/
PlayStopButton::PlayStopButton(MediaPlayer& _mediaPlayer) : MediaButton(_mediaPlayer) {
    observer_stopped = mediaPlayer.property_paused().observe([this](bool paused) {
        if (paused)
            action = media::start;
        else
            action = media::stop;
    });
}

void PlayStopButton::onBtnClick_post() {}

/** PlayPause ************************************************************************************/
PlayPauseButton::PlayPauseButton(MediaPlayer& _mediaPlayer) : MediaButton(_mediaPlayer) {
    observer_paused = mediaPlayer.property_paused().observe([this](bool paused) {
        if (paused)
            action = media::start;
        else
            action = media::pause;
    });
}

void PlayPauseButton::onBtnClick_post() {}

BtnGrpForwardBackward::BtnGrpForwardBackward() {
    set_orientation(Gtk::Orientation::HORIZONTAL);
    set_spacing(0);
    beginGroupBtn.set_image_from_icon_name("media-skip-backward");
    prevBtn.set_image_from_icon_name("media-seek-backward");
    nextBtn.set_image_from_icon_name("media-seek-forward");
    endGroupBtn.set_image_from_icon_name("media-skip-forward");
    append(beginGroupBtn);
    append(prevBtn);
    append(nextBtn);
    append(endGroupBtn);
}

void BtnGrpForwardBackward::setSensitive(bool sensitive) {
    beginGroupBtn.set_sensitive(sensitive);
    prevBtn.set_sensitive(sensitive);
    nextBtn.set_sensitive(sensitive);
    endGroupBtn.set_sensitive(sensitive);
}