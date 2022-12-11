#include <DataThread.h>
#include <SupplyAudio.h>
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
template <class Value_t> SpinBtnBox<Value_t>::SpinBtnBox() {
    set_orientation(Gtk::Orientation::VERTICAL);
    set_vexpand();
    adjustment = Gtk::Adjustment::create(0, 0, 0);
    adjustment->signal_value_changed().connect([this]() { currentValue = get_value(); });
    observe_value([this](Value_t value) { spinButton.set_value(value); });

    spinButton.set_adjustment(adjustment);
    append(spinButton);

    currentValue = get_value();
}

template <class Value_t> auto SpinBtnBox<Value_t>::get_value() const -> Value_t {
    if constexpr (std::is_integral_v<Value_t>)
        return spinButton.get_value_as_int();
    else
        return spinButton.get_value();
}

template <class Value_t>
void SpinBtnBox<Value_t>::observe_value(const std::function<void(Value_t)>& observer) {
    auto currentValue_observer = currentValue.observe(observer);
    observers.push(currentValue_observer);
}

template <class Value_t>
void SpinBtnBox<Value_t>::configure(double value,
                                    double lower,
                                    double upper,
                                    double step_increment,
                                    double page_increment,
                                    double page_size) {
    adjustment->configure(value, lower, upper, step_increment, page_increment, page_size);
    spinButton.set_adjustment(adjustment);
}

template <class Value_t> void SpinBtnBox<Value_t>::set_digits(int digits) {
    spinButton.set_digits(digits);
}

SpinBtnBoxCards::SpinBtnBoxCards(const std::string& label_str) : label(label_str, Gtk::Align::START) {
    adjustment->configure(800, 0, 2000, 1.0, 5.0, 0.0);
    prepend(label);
}

/* FragmentPlayBox *******************************************************************************/
FragmentPlayBox::FragmentPlayBox(const std::shared_ptr<MediaPlayer>& mediaPlayer_in)
    : mediaPlayer(mediaPlayer_in) {
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
            spinBtn_playStart.set_value(prev->value_end);
        }
    }));
    observers.push(spinBtn_playEnd.currentValue.observe([this](double val) {
        if (val < value_start) {
            spinBtn_playEnd.set_value(value_start);
        }
    }));
    spinBtn_playStart.set_digits(2);
    spinBtn_playEnd.set_digits(2);

    btn_playFirstFragment.set_image_from_icon_name("media-playback-start");
    btn_playLastFragment.set_image_from_icon_name("media-playback-start");
    btn_play.set_image_from_icon_name("media-repeat-single");
    btn_reverse.set_image_from_icon_name("media-seek-backward");
    append(btn_playFirstFragment);
    append(spinBtn_playStart);
    append(btn_playLastFragment);
    append(spinBtn_playEnd);
    append(btn_play);
    append(btn_reverse);
}

auto FragmentPlayBox::get_start() const -> double { return spinBtn_playStart.currentValue; }
auto FragmentPlayBox::get_end() const -> double { return spinBtn_playEnd.currentValue; }
void FragmentPlayBox::set_minimum(double min) { set_minmax(min, upper); }
void FragmentPlayBox::set_maximum(double max) { set_minmax(lower, max); }
void FragmentPlayBox::set_minmax(double min, double max) {
    lower = min;
    upper = max;
    configure();
}
void FragmentPlayBox::set_stepSize(double stepSize_in) {
    stepSize = stepSize_in;
    configure();
}
void FragmentPlayBox::configure() {
    spinBtn_playStart.configure(value_start, lower, upper, stepSize, stepSize * 5, 0.0);
    spinBtn_playEnd.configure(value_end, lower, upper, stepSize, stepSize * 5, 0.0);
}

/* SupplyAudio ***********************************************************************************/
SupplyAudio::SupplyAudio() {
    set_column_spacing(16);
    createMainCtrlBtnBox();
    DataThread::get().signal_paragraphFromIds_connect([this](auto&& paragraphs) {
        clearPage();
        fillPage(std::move(paragraphs));
    });

    setPlayPauseBtnIcon();
    btnPlayPause.signal_clicked().connect([this]() {
        mediaPlayer->play(mediaPlayer->is_paused());
        setPlayPauseBtnIcon();
    });
    btnPlayPause.set_halign(Gtk::Align::CENTER);

    attach(mainCtrlBtnBox, 0, 0, 3);
    cfgAudioFileObserver();

    observers.push(mediaPlayer->get_duration().observe([this](double) { fragment_adjust_min_max(); }));
    setUpCardAudioGroup(0);
}

void SupplyAudio::clearPage() {
    for (const auto& textDraw : textDrawContainer) {
        remove(*textDraw);
    }
    textDrawContainer.clear();
    for (const auto& fragmentPlayBox : fragmentPlayBoxesVisible) {
        remove(*fragmentPlayBox);
    }
    fragmentPlayBoxesVisible.clear();
}

void SupplyAudio::fillPage(std::vector<DataThread::paragraph_optional>&& paragraphs) {
    int row = 1;
    uint cardId = spinBtn_firstCard.currentValue;
    for (const auto& p : paragraphs) {
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

void SupplyAudio::cfgAudioFileObserver() {
    audioFile = DataThread::get().zikhronCfg.cfgMain.lastAudioFile.string();
    auto filenameObserver = audioFile.observe([this](const std::string& _filename) {
        if (!_filename.empty())
            DataThread::get().zikhronCfg.cfgMain.lastAudioFile = _filename;
        spdlog::info("Audio file selected: {}", _filename);
        mediaPlayer->openFile(_filename);
    });
    observers.push(filenameObserver);
}

auto SupplyAudio::addCard(const std::shared_ptr<markup::Paragraph>& paragraph, int row) -> int {
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

void SupplyAudio::addTextDraw(int column, int row, const std::string& markup) {
    auto textDraw = std::make_unique<TextDraw>();
    textDraw->setFontSize(textFontSize);
    textDraw->setSpacing(textSpacing);
    textDraw->setText(markup);
    textDraw->set_halign(Gtk::Align::START);
    attach(*textDraw, column, row);
    textDrawContainer.push_back(std::move(textDraw));
}
void SupplyAudio::setPlayPauseBtnIcon() {
    if (mediaPlayer->is_paused())
        btnPlayPause.set_image_from_icon_name("media-playback-start");
    else
        btnPlayPause.set_image_from_icon_name("media-playback-pause");
}

void SupplyAudio::createMainCtrlBtnBox() {
    mainCtrlBtnBox.set_orientation(Gtk::Orientation::HORIZONTAL);
    mainCtrlBtnBox.set_hexpand();
    mainCtrlBtnBox.set_vexpand(false);
    mainCtrlBtnBox.set_spacing(16);
    btnSave.set_image_from_icon_name("document-save");
    btnSave.signal_clicked().connect([this]() { saveCurrentAudioGroup(); });
    btnOpenFile.set_label("Open Audio");
    btnOpenFile.signal_clicked().connect([this]() { createFileChooserDialog(); });
    observers.push(spinBtn_firstCard.currentValue.observe([this](uint value) {
        if (spinBtn_lastCard.currentValue < value)
            spinBtn_lastCard.currentValue = value;
        if (spinBtn_firstCard.currentValue.get_old_value() > value + 20)
            spinBtn_lastCard.currentValue = value;
        requestCards();
    }));
    observers.push(spinBtn_lastCard.currentValue.observe([this](uint value) {
        if (spinBtn_firstCard.currentValue > value)
            spinBtn_firstCard.currentValue = value;
        if (spinBtn_lastCard.currentValue.get_old_value() < value - 20)
            spinBtn_firstCard.currentValue = value;
        requestCards();
    }));
    btnGroup_accuracy = std::make_unique<ButtonGroup>("1.0", "0.1", "0.01");
    btnGroup_accuracy->observe_active([this](uint digits) { fragment_adjust_stepsize(digits); });
    btnGroup_accuracy->setActive(0);
    mainCtrlBtnBox.append(btnSave);
    mainCtrlBtnBox.append(btnOpenFile);
    mainCtrlBtnBox.append(spinBtn_firstCard);
    mainCtrlBtnBox.append(spinBtn_lastCard);
    mainCtrlBtnBox.append(btnPlayPause);
    mainCtrlBtnBox.append(*btnGroup_accuracy);
}

void SupplyAudio::requestCards() {
    std::vector<uint> ids;
    ids.resize(spinBtn_lastCard.currentValue - spinBtn_firstCard.currentValue + 1);
    ranges::generate(ids, [n = int(spinBtn_firstCard.currentValue)]() mutable { return n++; });
    DataThread::get().requestCardFromIds(std::move(ids));
}

void SupplyAudio::createFileChooserDialog() {
    auto dialog = new Gtk::FileChooserDialog("Please choose a file", Gtk::FileChooser::Action::OPEN);
    Gtk::Window* parent = dynamic_cast<Gtk::Window*>(this->get_root());
    dialog->set_transient_for(*parent);

    if (fs::path audioFilePath = std::string(audioFile);
        not audioFilePath.empty() && fs::exists(audioFilePath)) {
        dialog->set_current_folder(Gio::File::create_for_path(audioFilePath.parent_path().string()));
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

void SupplyAudio::on_file_dialog_response(int response_id, Gtk::FileChooserDialog* dialog) {
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

void SupplyAudio::saveCurrentAudioGroup() {
    const auto first = first_fragmentPlayBox();
    const auto last = last_fragmentPlayBox();
    auto& cardId_audioFragment = cardAudio.cardId_audioFragment;
    ranges::transform(first,
                      last,
                      std::inserter(cardId_audioFragment, cardId_audioFragment.begin()),
                      [](const auto& cardId_fragmentPlayBox) -> std::pair<uint, AudioFragment> {
                          const auto& [cardId, fragmentPlayBox] = cardId_fragmentPlayBox;
                          return {cardId, {fragmentPlayBox->get_start(), fragmentPlayBox->get_end()}};
                      });
    cardAudio.audioFile = audioFile;
    cardAudioGroupDB.save(0, cardAudio);
}

void SupplyAudio::setUpCardAudioGroup(uint groupId) {
    auto cardAudioGroup = cardAudioGroupDB.get_cardAudioGroup(groupId);
    audioFile = cardAudioGroup.audioFile;
    const auto& cardId_audioFragments = cardAudioGroup.cardId_audioFragment;
    if (cardId_audioFragments.empty())
        return;
    spinBtn_firstCard.currentValue = cardId_audioFragments.begin()->first;
    spinBtn_lastCard.currentValue = cardId_audioFragments.rbegin()->first;
    for (const auto& [cardId, fragment] : cardId_audioFragments) {
        auto& fragmentPlayBox = fragmentPlayBoxes[cardId];
        if (!fragmentPlayBox)
            fragmentPlayBox = std::make_shared<FragmentPlayBox>(mediaPlayer);
        fragmentPlayBox->value_start = fragment.start;
        fragmentPlayBox->value_end = fragment.end;
    }
}

void SupplyAudio::fragment_adjust_min_max() {
    for_each_fragmentPlayBox([this](const FragmentPlayBoxPtr& fragmentPlayBox) {
        fragmentPlayBox->set_minimum(0.0);
        fragmentPlayBox->set_maximum(mediaPlayer->get_duration());
    });
}

void SupplyAudio::fragment_adjust_stepsize(uint digits) {
    for_each_fragmentPlayBox([this, digits](const FragmentPlayBoxPtr& fragmentPlayBox) {
        fragmentPlayBox->set_stepSize(std::pow(0.1, digits));
    });
}

auto SupplyAudio::first_fragmentPlayBox() const -> decltype(ranges::begin(fragmentPlayBoxes)) {
    auto first = ranges::find_if(
        fragmentPlayBoxes,
        [this](uint cardId) { return cardId >= spinBtn_firstCard.currentValue; },
        &decltype(fragmentPlayBoxes)::value_type::first);
    return first;
}

auto SupplyAudio::last_fragmentPlayBox() const -> decltype(std::ranges::begin(fragmentPlayBoxes)) {
    auto last = ranges::find_if(
        fragmentPlayBoxes,
        [this](uint cardId) { return cardId > spinBtn_lastCard.currentValue; },
        &decltype(fragmentPlayBoxes)::value_type::first);
    return last;
}

void SupplyAudio::fragment_adjacent_connect() {
    FragmentPlayBoxPtr prev;
    for (auto& fragmentPlayBox : fragmentPlayBoxesVisible) {
        fragmentPlayBox->prev_fragmentPlayBox = prev;
        prev = fragmentPlayBox;
    }
}

void SupplyAudio::for_each_fragmentPlayBox(std::function<void(FragmentPlayBoxPtr)> fun) {
    const auto first = first_fragmentPlayBox();
    const auto last = last_fragmentPlayBox();
    ranges::for_each(first, last, fun, &decltype(fragmentPlayBoxes)::value_type::second);
}