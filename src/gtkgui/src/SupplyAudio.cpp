#include <DataThread.h>
#include <SupplyAudio.h>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <ranges>
#include <span>
#include <type_traits>

namespace ranges = std::ranges;
namespace fs = std::filesystem;

/* SpinBtnBox ************************************************************************************/
template<class Value_t>
SpinBtnBox<Value_t>::SpinBtnBox()
{
    set_orientation(Gtk::Orientation::VERTICAL);
    set_vexpand();
    adjustment = Gtk::Adjustment::create(0, 0, 0);
    adjustment->signal_value_changed().connect([this]() { currentValue = get_value(); });
    observe_value([this](Value_t value) { spinButton.set_value(value); });

    spinButton.set_adjustment(adjustment);
    append(spinButton);

    currentValue = get_value();
}

template<class Value_t>
auto SpinBtnBox<Value_t>::get_value() const -> Value_t
{
    if constexpr (std::is_integral_v<Value_t>)
        return spinButton.get_value_as_int();
    else
        return spinButton.get_value();
}

template<class Value_t>
void SpinBtnBox<Value_t>::observe_value(const std::function<void(Value_t)>& observer)
{
    auto currentValue_observer = currentValue.observe(observer);
    observers.push(currentValue_observer);
}

template<class Value_t>
void SpinBtnBox<Value_t>::configure(double value,
                                    double lower,
                                    double upper,
                                    double step_increment,
                                    double page_increment,
                                    double page_size)
{
    adjustment->configure(value, lower, upper, step_increment, page_increment, page_size);
    spinButton.set_adjustment(adjustment);
}

template<class Value_t>
void SpinBtnBox<Value_t>::set_value(Value_t value)
{
    changeBySetValue = true;
    spinButton.set_value(value);
}

template<class Value_t>
void SpinBtnBox<Value_t>::set_digits(int digits)
{
    spinButton.set_digits(digits);
}

SpinBtnBoxCards::SpinBtnBoxCards(const std::string& label_str)
    : label(label_str, Gtk::Align::START)
{
    adjustment->configure(0, 0, 65536, 1.0, 5.0, 0.0);
    prepend(label);
}

/* PlayBox ***************************************************************************************/
PlayBox::PlayBox(std::shared_ptr<MediaPlayer> _mediaPlayer)
    : mediaPlayer(_mediaPlayer), playBtn(_mediaPlayer)
{
    set_orientation(Gtk::Orientation::HORIZONTAL);
    set_expand();
    append(grid);
    lbl_timePos.set_halign(Gtk::Align::START);

    grid.attach(lbl_timePos, 1, 0);
    grid.attach(playBtn, 0, 1);
    grid.attach(slider, 1, 1);
    slider.set_expand();
    slider.setSpacing(3);

    observers.push(mediaPlayer->property_timePos().observe([this](double val) {
        double length = mediaPlayer->property_duration();
        if (length == 0.)
            return;
        slider.setProgress(val / length);
        lbl_timePos.set_label(fmt::format("{:.2f}", val + 0.001));
    }));
    slider.signal_clickProgress_connect([this](double progress) {
        mediaPlayer->play_fragment(mediaPlayer->property_duration() * progress,
                                   mediaPlayer->property_duration());
    });
    playBtn.signal_start_connect([](std::shared_ptr<MediaPlayer> mediaPlayer_in) {
        mediaPlayer_in->play_fragment(mediaPlayer_in->property_timePos(),
                                      mediaPlayer_in->property_duration());
    });
    playBtn.signal_pause_connect([](std::shared_ptr<MediaPlayer> mediaPlayer_in) { mediaPlayer_in->pause(); });
}

/* FragmentPlayBox *******************************************************************************/
FragmentPlayBox::FragmentPlayBox(std::shared_ptr<MediaPlayer> _mediaPlayer)
    : mediaPlayer(std::move(_mediaPlayer))
{
    set_spacing(8);
    set_orientation(Gtk::Orientation::HORIZONTAL);
    // set_hexpand();
    // set_vexpand(false);
    set_expand(false);
    observers.push(spinBtn_playStart.currentValue.observe([this](double val) {
        auto prev = prev_fragmentPlayBox.lock();
        if (not prev)
            return;

        if (val < prev->value_end) {
            spinBtn_playStart.set_value(
                    std::max<double>(mediaPlayer->property_timePos(), prev->value_end));
        }
    }));
    observers.push(spinBtn_playEnd.currentValue.observe([this](double val) {
        if (val < value_start) {
            spinBtn_playEnd.set_value(std::max<double>(mediaPlayer->property_timePos(), value_start));
        }
    }));
    spinBtn_playStart.set_digits(2);
    spinBtn_playEnd.set_digits(2);

    btn_playFirstFragment.signal_start_connect([this](std::shared_ptr<MediaPlayer> player) {
        double startValue = spinBtn_playStart.currentValue;
        player->play_fragment(startValue, startValue + 3.);
    });
    btn_playFirstFragment.signal_stop_connect([](std::shared_ptr<MediaPlayer> player) { player->pause(); });
    btn_playLastFragment.signal_start_connect([this](std::shared_ptr<MediaPlayer> player) {
        double startValue = std::max<double>(spinBtn_playEnd.currentValue - 1.7,
                                             spinBtn_playStart.currentValue);
        double endValue = spinBtn_playEnd.currentValue;
        player->play_fragment(startValue, endValue);
    });
    btn_playLastFragment.signal_stop_connect([](std::shared_ptr<MediaPlayer> player) { player->pause(); });
    btn_play.signal_start_connect([this](std::shared_ptr<MediaPlayer> player) {
        double startValue = spinBtn_playStart.currentValue;
        double endValue = spinBtn_playEnd.currentValue;
        player->play_fragment(startValue, endValue);
    });
    btn_play.signal_stop_connect([](std::shared_ptr<MediaPlayer> player) { player->pause(); });
    append(btn_playFirstFragment);
    append(spinBtn_playStart);
    append(btn_playLastFragment);
    append(spinBtn_playEnd);
    append(btn_play);
}

auto FragmentPlayBox::get_start() const -> double
{
    return spinBtn_playStart.currentValue;
}
auto FragmentPlayBox::get_end() const -> double
{
    return spinBtn_playEnd.currentValue;
}
void FragmentPlayBox::set_minimum(double min)
{
    set_minmax(min, upper);
}
void FragmentPlayBox::set_maximum(double max)
{
    set_minmax(lower, max);
}
void FragmentPlayBox::set_minmax(double min, double max)
{
    lower = min;
    upper = max;
    configure();
}
void FragmentPlayBox::set_stepSize(double stepSize_in)
{
    stepSize = stepSize_in;
    configure();
}
void FragmentPlayBox::configure()
{
    spinBtn_playStart.configure(value_start, lower, upper, stepSize, stepSize * 5, 0.0);
    spinBtn_playEnd.configure(value_end, lower, upper, stepSize, stepSize * 5, 0.0);
}

/* SupplyAudio ***********************************************************************************/
SupplyAudio::SupplyAudio()
    : mediaPlayer{std::make_shared<MediaPlayer>()}
{
    set_column_spacing(16);
    createMainCtrlBtnBox();
    DataThread::get().signal_paragraphFromIds_connect([this](auto&& paragraphs) {
        clearPage();
        fillPage(std::move(paragraphs));
    });

    attach(mainCtrlBtnBox, 0, 0, 4);
    cfgAudioFileObserver();

    observers.push(mediaPlayer->property_duration().observe([this](double /*duration*/) {
        fragment_adjust_min_max();
        setUpFragmentPlayBoxes(spinBtn_audioGroup.currentValue);
    }));
}

void SupplyAudio::clearPage()
{
    for (const auto& textDraw : textDrawContainer) {
        remove(*textDraw);
    }
    textDrawContainer.clear();
    for (const auto& fragmentPlayBox : fragmentPlayBoxesVisible) {
        remove(*fragmentPlayBox);
    }
    fragmentPlayBoxesVisible.clear();
}

void SupplyAudio::fillPage(std::vector<DataThread::paragraph_optional>&& paragraphs)
{
    uint cardId = spinBtn_firstCard.currentValue;
    for (int row{1}; const auto& p : paragraphs) {
        if (p.has_value()) {
            if (!fragmentPlayBoxes[cardId])
                fragmentPlayBoxes[cardId] = std::make_shared<FragmentPlayBox>(mediaPlayer);
            attach(*fragmentPlayBoxes[cardId], 3, row);
            fragmentPlayBoxesVisible.push_back(fragmentPlayBoxes[cardId]);
            row = addCard(p.value(), row);
        } else {
            if (fragmentPlayBoxes.contains(cardId)) {
                fragmentPlayBoxes.erase(cardId);
            }
        }
        cardId++;
    }
    fragment_adjust_min_max();
    fragment_adjust_stepsize(btnGroup_accuracy->getActive());
    fragment_adjacent_connect();
}

void SupplyAudio::cfgAudioFileObserver()
{
    audioFile = DataThread::get().zikhronCfg.cfgMain.lastAudioFile.string();
    auto filenameObserver = audioFile.observe([this](const std::string& _filename) {
        if (!_filename.empty())
            DataThread::get().zikhronCfg.cfgMain.lastAudioFile = _filename;
        spdlog::info("Audio file selected: {}", _filename);
        mediaPlayer->openFile(_filename);
    });
    observers.push(filenameObserver);
}

auto SupplyAudio::addCard(const std::shared_ptr<markup::Paragraph>& paragraph, int row) -> int
{
    int textDrawIndex = 0;
    if (paragraph->getFragments().size() == 1)
        textDrawIndex++;
    for (const auto& fragment : paragraph->getFragments()) {
        addTextDraw(textDrawIndex % 2, row, fragment);
        row += textDrawIndex % 2;
        textDrawIndex++;
    }
    return row;
}

void SupplyAudio::addTextDraw(int column, int row, const std::string& markup)
{
    auto textDraw = std::make_unique<TextDraw>();
    textDraw->setFontSize(textFontSize);
    textDraw->setSpacing(textSpacing);
    textDraw->setText(markup);
    textDraw->set_halign(Gtk::Align::START);
    attach(*textDraw, column, row);
    textDrawContainer.push_back(std::move(textDraw));
}

void SupplyAudio::createMainCtrlBtnBox()
{
    mainCtrlBtnBox.set_orientation(Gtk::Orientation::HORIZONTAL);
    mainCtrlBtnBox.set_hexpand();
    mainCtrlBtnBox.set_vexpand(false);
    mainCtrlBtnBox.set_spacing(16);
    btnSave.set_image_from_icon_name("document-save");
    btnSave.signal_clicked().connect([this]() {
        auto cag = getCurrentAudioGroup();
        cardAudioGroupDB.save(spinBtn_audioGroup.currentValue, cag);
    });
    btnStudyGroup.set_label("study group");
    btnStudyGroup.signal_clicked().connect([this]() {
        auto cag = getCurrentAudioGroup();
        if (cag.cardId_audioFragment.empty()) {
            return;
        }
        auto& [key, _] = *cag.cardId_audioFragment.begin();
        DataThread::get().requestCard({key});
    });

    btnOpenAudioFile.set_image_from_icon_name("media-record");
    btnOpenAudioFile.signal_clicked().connect([this]() { createFileChooserDialog(); });
    btnNewAudioCardGroup.set_image_from_icon_name("document-new");
    btnNewAudioCardGroup.signal_clicked().connect([this]() { createNewAudioCardGroup(); });
    observers.push(spinBtn_audioGroup.currentValue.observe([this](uint value) {
        auto cag = getCurrentAudioGroup();
        cardAudioGroupDB.insert(spinBtn_audioGroup.currentValue.get_old_value(), cag);

        uint newVal = 0;
        if (spinBtn_audioGroup.currentValue.get_old_value() < value)
            newVal = cardAudioGroupDB.nextOrThisGroupId(value);
        else
            newVal = cardAudioGroupDB.prevOrThisGroupId(value);
        spinBtn_audioGroup.set_value(newVal);
        setUpCardAudioGroup(newVal);
    }));

    observers.push(spinBtn_firstCard.currentValue.observe([this](uint value) {
        if (not spinBtn_firstCard.changeBySetValue) {
            if (spinBtn_lastCard.currentValue < value)
                spinBtn_lastCard.currentValue = value;
            if (spinBtn_firstCard.currentValue.get_old_value() > value + 10)
                spinBtn_lastCard.currentValue = value;
        }
        spinBtn_firstCard.changeBySetValue = false;
        requestCards();
    }));
    observers.push(spinBtn_lastCard.currentValue.observe([this](uint value) {
        if (not spinBtn_lastCard.changeBySetValue) {
            if (spinBtn_firstCard.currentValue > value) {
                spinBtn_firstCard.currentValue = value;
            }
            if (spinBtn_lastCard.currentValue.get_old_value() + 10 < value) {
                spinBtn_firstCard.currentValue = value;
            }
        }
        spinBtn_lastCard.changeBySetValue = false;
        requestCards();
    }));
    btnGroup_accuracy = std::make_unique<ButtonGroup>("1.0", "0.1", "0.01");
    btnGroup_accuracy->observe_active([this](uint digits) { fragment_adjust_stepsize(digits); });
    btnGroup_accuracy->setActive(0);
    mainCtrlBtnBox.append(btnSave);
    mainCtrlBtnBox.append(btnStudyGroup);
    mainCtrlBtnBox.append(spinBtn_audioGroup);
    mainCtrlBtnBox.append(btnNewAudioCardGroup);
    mainCtrlBtnBox.append(btnOpenAudioFile);
    mainCtrlBtnBox.append(spinBtn_firstCard);
    mainCtrlBtnBox.append(spinBtn_lastCard);
    mainCtrlBtnBox.append(*btnGroup_accuracy);
    mainCtrlBtnBox.append(playBox);
}

void SupplyAudio::requestCards()
{
    std::vector<uint> ids;
    ids.resize(spinBtn_lastCard.currentValue - spinBtn_firstCard.currentValue + 1);
    ranges::generate(ids, [n = int(spinBtn_firstCard.currentValue)]() mutable { return n++; });
    DataThread::get().requestCardFromIds(std::move(ids));
}

void SupplyAudio::createFileChooserDialog()
{
    auto* dialog = new Gtk::FileChooserDialog("Please choose a file", Gtk::FileChooser::Action::OPEN);
    auto* parent = dynamic_cast<Gtk::Window*>(this->get_root());
    dialog->set_transient_for(*parent);
    if (fs::path audioFilePath = std::string(audioFile);
        not audioFilePath.empty() && fs::exists(audioFilePath)) {
        dialog->set_current_folder(Gio::File::create_for_path(audioFilePath.parent_path().string()));
    } else if (not oldValidAudiofilePath.empty()) {
        dialog->set_current_folder(
                Gio::File::create_for_path(oldValidAudiofilePath.parent_path().string()));
    }
    dialog->set_modal(true);
    dialog->signal_response().connect(
            sigc::bind(sigc::mem_fun(*this, &SupplyAudio::on_file_dialog_response), dialog));

    // Add response buttons to the dialog:
    dialog->add_button("_Cancel", Gtk::ResponseType::CANCEL);
    dialog->add_button("_Open", Gtk::ResponseType::OK);

    // Add filters, so that only certain file types can be selected:
    auto filter_audio = Gtk::FileFilter::create();
    filter_audio->set_name("Audio, *.mp3");
    filter_audio->add_mime_type("audio/mpeg");
    dialog->add_filter(filter_audio);

    auto filter_any = Gtk::FileFilter::create();
    filter_any->set_name("Any files");
    filter_any->add_pattern("*");
    dialog->add_filter(filter_any);

    // Show the dialog and wait for a user response:
    dialog->show();
}

void SupplyAudio::on_file_dialog_response(int response_id, Gtk::FileChooserDialog* dialog)
{
    switch (response_id) {
    case Gtk::ResponseType::OK: {
        audioFile = dialog->get_file()->get_path();
        break;
    }
    default: {
        spdlog::info("No file selected.");
        break;
    }
    }
    delete dialog;
}

auto SupplyAudio::getCurrentAudioGroup() const -> CardAudioGroup
{
    const auto first = first_fragmentPlayBox();
    const auto last = last_fragmentPlayBox();
    CardAudioGroup cardAudio;
    auto& cardId_audioFragment = cardAudio.cardId_audioFragment;
    ranges::transform(first,
                      last,
                      std::inserter(cardId_audioFragment, cardId_audioFragment.begin()),
                      [](const auto& cardId_fragmentPlayBox) -> std::pair<uint, AudioFragment> {
                          const auto& [cardId, fragmentPlayBox] = cardId_fragmentPlayBox;
                          return {cardId, {fragmentPlayBox->get_start(), fragmentPlayBox->get_end()}};
                      });
    cardAudio.audioFile = audioFile;

    return cardAudio;
}

void SupplyAudio::setUpCardAudioGroup(uint groupId)
{
    spinBtn_audioGroup.set_value(groupId);

    auto cardAudioGroup = cardAudioGroupDB.get_cardAudioGroup(groupId);
    audioFile = cardAudioGroup.audioFile;

    if (not cardAudioGroup.audioFile.empty() && fs::exists(cardAudioGroup.audioFile))
        oldValidAudiofilePath = cardAudioGroup.audioFile;
    const auto& cardId_audioFragments = cardAudioGroup.cardId_audioFragment;
    if (cardId_audioFragments.empty()) {
        spinBtn_firstCard.set_value(0);
        spinBtn_lastCard.set_value(0);
        return;
    }
    spinBtn_firstCard.set_value(cardId_audioFragments.begin()->first);
    spinBtn_lastCard.set_value(cardId_audioFragments.rbegin()->first);
    setUpFragmentPlayBoxes(groupId);
}

void SupplyAudio::setUpFragmentPlayBoxes(uint groupId)
{
    auto cardAudioGroup = cardAudioGroupDB.get_cardAudioGroup(groupId);
    const auto& cardId_audioFragments = cardAudioGroup.cardId_audioFragment;

    for (const auto& [cardId, fragment] : cardId_audioFragments) {
        auto& fragmentPlayBox = fragmentPlayBoxes[cardId];
        if (!fragmentPlayBox)
            fragmentPlayBox = std::make_shared<FragmentPlayBox>(mediaPlayer);
        fragmentPlayBox->value_start = fragment.start;
        fragmentPlayBox->value_end = fragment.end;
    }
}

void SupplyAudio::createNewAudioCardGroup()
{
    uint groupId = cardAudioGroupDB.newCardAudioGroup();
    spinBtn_audioGroup.set_value(groupId);
}

void SupplyAudio::fragment_adjust_min_max()
{
    for_each_fragmentPlayBox([this](const FragmentPlayBoxPtr& fragmentPlayBox) {
        fragmentPlayBox->set_minimum(0.0);
        fragmentPlayBox->set_maximum(mediaPlayer->property_duration());
    });
}

void SupplyAudio::fragment_adjust_stepsize(uint digits)
{
    for_each_fragmentPlayBox([this, digits](const FragmentPlayBoxPtr& fragmentPlayBox) {
        fragmentPlayBox->set_stepSize(std::pow(0.1, digits));
    });
}

auto SupplyAudio::first_fragmentPlayBox() const -> decltype(ranges::begin(fragmentPlayBoxes))
{
    auto first = ranges::find_if(
            fragmentPlayBoxes,
            [this](uint cardId) { return cardId >= spinBtn_firstCard.currentValue; },
            &decltype(fragmentPlayBoxes)::value_type::first);
    return first;
}

auto SupplyAudio::last_fragmentPlayBox() const -> decltype(std::ranges::begin(fragmentPlayBoxes))
{
    auto last = ranges::find_if(
            fragmentPlayBoxes,
            [this](uint cardId) { return cardId > spinBtn_lastCard.currentValue; },
            &decltype(fragmentPlayBoxes)::value_type::first);
    return last;
}

void SupplyAudio::fragment_adjacent_connect()
{
    FragmentPlayBoxPtr prev;
    for (auto& fragmentPlayBox : fragmentPlayBoxesVisible) {
        fragmentPlayBox->prev_fragmentPlayBox = prev;
        prev = fragmentPlayBox;
    }
}

void SupplyAudio::for_each_fragmentPlayBox(std::function<void(FragmentPlayBoxPtr)> fun)
{
    const auto first = first_fragmentPlayBox();
    const auto last = last_fragmentPlayBox();
    ranges::for_each(first, last, fun, &decltype(fragmentPlayBoxes)::value_type::second);
}
