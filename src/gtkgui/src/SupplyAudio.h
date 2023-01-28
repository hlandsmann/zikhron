#pragma once

#include <ButtonGroup.h>
#include <MediaButtons.h>
#include <MediaSlider.h>
#include <NotebookPage.h>
#include <TextDraw.h>
#include <gtkmm.h>
#include <multimedia/CardAudioGroup.h>
#include <multimedia/MediaPlayer.h>
#include <utils/Property.h>
#include <filesystem>
#include <functional>
#include <ranges>

template <class Value_t> class SpinBtnBox : public Gtk::Box {
public:
    SpinBtnBox();
    virtual ~SpinBtnBox() = default;
    void configure(double value,
                   double lower,
                   double upper,
                   double step_increment,
                   double page_increment,
                   double page_size);
    utl::Property<Value_t> currentValue = 0;
    void set_value(Value_t value);
    void set_digits(int digits);
    bool changeBySetValue = false;

protected:
    void observe_value(const std::function<void(Value_t)>& observer);
    auto get_value() const -> Value_t;

    utl::ObserverCollection observers;

    Gtk::SpinButton spinButton;
    Glib::RefPtr<Gtk::Adjustment> adjustment;
};

class SpinBtnBoxCards : public SpinBtnBox<uint> {
public:
    SpinBtnBoxCards(const std::string& label_str);

private:
    Gtk::Label label;
};

class SpinBtnBoxPlay : public SpinBtnBox<double> {
public:
    SpinBtnBoxPlay() = default;
};

class PlayBox : public Gtk::Box {
public:
    PlayBox(MediaPlayer&);

private:
    utl::ObserverCollection observers;
    MediaPlayer& mediaPlayer;
    Gtk::Grid grid;
    PlayPauseButton playBtn;
    MediaSlider slider;
    Gtk::Label lbl_timePos;
};

class FragmentPlayBox : public Gtk::Box {
public:
    FragmentPlayBox(MediaPlayer& mediaPlayer);
    void set_minmax(double min, double max);
    void set_minimum(double min);
    void set_maximum(double max);
    void set_stepSize(double stepSize);
    auto get_start() const -> double;
    auto get_end() const -> double;

private:
    void configure();
    MediaPlayer& mediaPlayer;

    SpinBtnBoxPlay spinBtn_playStart;
    SpinBtnBoxPlay spinBtn_playEnd;
    PlayStopButton btn_playFirstFragment{mediaPlayer};
    PlayStopButton btn_playLastFragment{mediaPlayer};
    PlayStopButton btn_play{mediaPlayer};
    utl::ObserverCollection observers;

    double lower = 0;
    double upper = 0;
    double stepSize = 0.1;

public:
    std::weak_ptr<FragmentPlayBox> prev_fragmentPlayBox;
    utl::Property<double>& value_start = spinBtn_playStart.currentValue;
    utl::Property<double>& value_end = spinBtn_playEnd.currentValue;
};

class SupplyAudio : public Gtk::Grid, public NotebookPage {
    using TextDrawPtr = std::unique_ptr<TextDraw>;
    using FragmentPlayBoxPtr = std::shared_ptr<FragmentPlayBox>;
    constexpr static int textFontSize = 20;
    constexpr static int textSpacing = 5;

public:
    SupplyAudio();

private:
    void clearPage();
    void fillPage(std::vector<DataThread::paragraph_optional>&& paragraphs);
    void createMainCtrlBtnBox();
    auto addCard(const std::shared_ptr<markup::Paragraph>& paragraph, int row) -> int;
    void addTextDraw(int column, int row, const std::string& markup);
    void requestCards();
    void createFileChooserDialog();
    void on_file_dialog_response(int response_id, Gtk::FileChooserDialog* dialog);
    void cfgAudioFileObserver();
    auto getCurrentAudioGroup() const -> CardAudioGroup;
    void setUpCardAudioGroup(uint groupId);
    void setUpFragmentPlayBoxes(uint groupId);
    void createNewAudioCardGroup();

    std::map<uint, FragmentPlayBoxPtr> fragmentPlayBoxes;
    auto first_fragmentPlayBox() const -> decltype(std::ranges::begin(fragmentPlayBoxes));
    auto last_fragmentPlayBox() const -> decltype(std::ranges::begin(fragmentPlayBoxes));
    void fragment_adjust_min_max();
    void fragment_adjust_stepsize(uint digits);
    void fragment_adjacent_connect();
    void for_each_fragmentPlayBox(std::function<void(FragmentPlayBoxPtr)> fun);

    utl::ObserverCollection observers;
    utl::Property<std::string> audioFile;

    std::vector<TextDrawPtr> textDrawContainer;
    std::vector<FragmentPlayBoxPtr> fragmentPlayBoxesVisible;

    std::filesystem::path oldValidAudiofilePath;  // configuring file chooser dialogue

    MediaPlayer mediaPlayer{};
    Gtk::Button btnSave;
    Gtk::Button btnOpenAudioFile;
    Gtk::Button btnNewAudioCardGroup;
    PlayBox playBox{mediaPlayer};
    std::unique_ptr<ButtonGroup> btnGroup_accuracy;

    Gtk::Box mainCtrlBtnBox;
    SpinBtnBoxCards spinBtn_audioGroup = SpinBtnBoxCards("Audio Group");
    SpinBtnBoxCards spinBtn_firstCard = SpinBtnBoxCards("First Card");
    SpinBtnBoxCards spinBtn_lastCard = SpinBtnBoxCards("Last Card");

    CardAudioGroupDB& cardAudioGroupDB = CardAudioGroupDB::get();
};