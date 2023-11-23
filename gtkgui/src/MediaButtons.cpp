#include <MediaButtons.h>

MediaButton::MediaButton(std::shared_ptr<MediaPlayer> _mediaPlayer, Gtk::Orientation orientation)
    : mediaPlayer(std::move(_mediaPlayer))
{
    set_orientation(orientation);
    append(button);
    observers.push(action.observe([this](const media& _action) {
        switch (_action) {
        case media::start:
            button.set_image_from_icon_name("media-playback-start");
            break;
        case media::stop:
            button.set_image_from_icon_name("media-playback-stop");
            break;
        case media::pause:
            button.set_image_from_icon_name("media-playback-pause");
            break;
        }
    }));
    button.signal_clicked().connect([this]() { onBtnClick(); });
}
MediaButton::MediaButton(std::shared_ptr<MediaPlayer> _mediaPlayer)
    : MediaButton(_mediaPlayer, Gtk::Orientation::VERTICAL) {}
void MediaButton::signal_start_connect(std::function<void(std::shared_ptr<MediaPlayer>)> signal_fun)
{
    signal_start = signal_fun;
}

void MediaButton::signal_pause_connect(std::function<void(std::shared_ptr<MediaPlayer>)> signal_fun)
{
    signal_pause = signal_fun;
}

void MediaButton::signal_stop_connect(std::function<void(std::shared_ptr<MediaPlayer>)> signal_fun)
{
    signal_stop = signal_fun;
}

auto MediaButton::getMediaPlayer() const -> const std::shared_ptr<MediaPlayer>&
{
    return mediaPlayer;
}

void MediaButton::onBtnClick()
{
    switch (action) {
    case media::start:
        if (signal_start) {
            signal_start(mediaPlayer);
        }
        break;
    case media::stop:
        if (signal_stop) {
            signal_stop(mediaPlayer);
        }
        break;
    case media::pause:
        if (signal_pause) {
            signal_pause(mediaPlayer);
        }
        break;
    }
    onBtnClick_post();
    action = nextAction;
}

/** PlayStop *************************************************************************************/
PlayStopButton::PlayStopButton(std::shared_ptr<MediaPlayer> _mediaPlayer)
    : MediaButton(_mediaPlayer)
{
    observer_stopped = getMediaPlayer()->property_paused().observe([this](bool paused) {
        if (paused) {
            action = media::start;
        } else {
            action = media::stop;
        }
    });
}

void PlayStopButton::onBtnClick_post() {}

/** PlayPause ************************************************************************************/
PlayPauseButton::PlayPauseButton(std::shared_ptr<MediaPlayer> _mediaPlayer)
    : MediaButton(_mediaPlayer)
{
    observer_paused = getMediaPlayer()->property_paused().observe([this](bool paused) {
        if (paused) {
            action = media::start;
        } else {
            action = media::pause;
        }
    });
}

void PlayPauseButton::onBtnClick_post() {}

BtnGrpForwardBackward::BtnGrpForwardBackward()
{
    set_orientation(Gtk::Orientation::HORIZONTAL);
    set_spacing(0);
    skipBackwardBtn.set_image_from_icon_name("media-skip-backward");
    seekBackwardBtn.set_image_from_icon_name("media-seek-backward");
    seekForwardBtn.set_image_from_icon_name("media-seek-forward");
    skipForwardBtn.set_image_from_icon_name("media-skip-forward");
    append(skipBackwardBtn);
    append(seekBackwardBtn);
    append(seekForwardBtn);
    append(skipForwardBtn);
}

void BtnGrpForwardBackward::setSensitive(bool sensitive)
{
    skipBackwardBtn.set_sensitive(sensitive);
    seekBackwardBtn.set_sensitive(sensitive);
    seekForwardBtn.set_sensitive(sensitive);
    skipForwardBtn.set_sensitive(sensitive);
}

void BtnGrpForwardBackward::setIndividualSensitivity(bool sensitive_skipBackwardBtn,
                                                     bool sensitive_seekBackwardBtn,
                                                     bool sensitive_seekForwardBtn,
                                                     bool sensitive_skipForwardBtn)
{
    skipBackwardBtn.set_sensitive(sensitive_skipBackwardBtn);
    seekBackwardBtn.set_sensitive(sensitive_seekBackwardBtn);
    seekForwardBtn.set_sensitive(sensitive_seekForwardBtn);
    skipForwardBtn.set_sensitive(sensitive_skipForwardBtn);
}

void BtnGrpForwardBackward::skipBackwardClick(const std::function<void()>& cb_fun)
{
    skipBackwardBtn.signal_clicked().connect(cb_fun);
}

void BtnGrpForwardBackward::seekBackwardClick(const std::function<void()>& cb_fun)
{
    seekBackwardBtn.signal_clicked().connect(cb_fun);
}

void BtnGrpForwardBackward::seekForwardClick(const std::function<void()>& cb_fun)
{
    seekForwardBtn.signal_clicked().connect(cb_fun);
}

void BtnGrpForwardBackward::skipForwardClick(const std::function<void()>& cb_fun)
{
    skipForwardBtn.signal_clicked().connect(cb_fun);
}
