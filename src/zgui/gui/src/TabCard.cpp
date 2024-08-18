#include "TabCard.h"

#include <DisplayAnnotation.h>
#include <DisplayText.h>
#include <DisplayVideo.h>
#include <DisplayVocables.h>
#include <ImGuiFileDialog/ImGuiFileDialog.h>
#include <VocableOverlay.h>
#include <context/imglog.h>
#include <database/CardDB.h>
#include <database/CardPackDB.h>
#include <database/Track.h>
#include <database/VideoDB.h>
#include <database/VideoSet.h>
#include <database/WordDB.h>
#include <misc/Identifier.h>
#include <misc/TokenizationChoice.h>
#include <multimedia/MpvWrapper.h>
#include <spaced_repetition/AsyncTreeWalker.h>
#include <spaced_repetition/CardMeta.h>
#include <spdlog/spdlog.h>
#include <utils/Algorithm.h>
#include <widgets/Box.h>
#include <widgets/Button.h>
#include <widgets/ImageButton.h>
#include <widgets/Layer.h>
#include <widgets/MediaSlider.h>
#include <widgets/Overlay.h>
#include <widgets/Separator.h>
#include <widgets/TextTokenSeq.h>
#include <widgets/ToggleButtonGroup.h>
#include <widgets/Video.h>
#include <widgets/Window.h>
#include <widgets/detail/Widget.h>

#include <algorithm>
#include <initializer_list>
#include <kocoro/kocoro.hpp>
#include <memory>
#include <optional>
#include <string>
#include <utility>

namespace gui {

TabCard::TabCard(std::shared_ptr<kocoro::SynchronousExecutor> _synchronousExecutor,
                 std::shared_ptr<sr::AsyncTreeWalker> _asyncTreeWalker,
                 std::unique_ptr<DisplayVideo> _displayVideo,
                 std::unique_ptr<multimedia::MpvWrapper> _mpvAudio)
    : executor{std::move(_synchronousExecutor)}
    , signalCardBox{executor->makePersistentSignal<BoxPtr>()}
    , signalProceed{executor->makeVolatileSignal<Proceed>()}
    , signalAnnotationDone{executor->makeVolatileSignal<TokenizationChoice>()}
    , displayVideo{std::move(_displayVideo)}
    , mpvAudio{std::move(_mpvAudio)}
    , mpvVideo{displayVideo->getMpv()}
{
    executor->startCoro(feedingTask(std::move(_asyncTreeWalker)));
}

void TabCard::setUp(widget::Layer& layer)
{
    using namespace widget::layout;
    auto box = layer.add<widget::Box>(Align::start, widget::Orientation::vertical);
    box->setName("DisplayCard_box");
    boxId = box->getWidgetId();

    auto& cardWindow = *box->add<widget::Window>(Align::start, width_expand, height_expand, "card_text");
    auto& ctrlWindow = *box->add<widget::Window>(Align::end, ExpandType::width_expand, ExpandType::height_fixed, "card_ctrl");
    setupCardWindow(cardWindow);
    setupCtrlWindow(ctrlWindow);
}

void TabCard::displayOnLayer(widget::Layer& layer)
{
    auto& box = layer.getWidget<widget::Box>(boxId);
    box.start();
    doCardWindow(box.next<widget::Window>());
    doCtrlWindow(box.next<widget::Window>());
}

void TabCard::slot_playVideoSet(database::VideoSetPtr videoSet)
{
    auto cardDB = dataBase->getCardDB();
    const auto& videos = videoSet->getVideos();

    track = cardDB->getTrackFromVideo(videos.begin()->second);

    signalProceed->set(Proceed::nextTrack);
    mode = Mode::story;
    // mpvVideo->openFile(videoSet->getVideo()->getVideoFile());
    // mpvVideo->play();
}

auto TabCard::feedingTask(std::shared_ptr<sr::AsyncTreeWalker> asyncTreeWalker) -> kocoro::Task<>
{
    using namespace widget::layout;
    auto cardBox = co_await *signalCardBox;
    auto cardLayer = cardBox->add<widget::Layer>(Align::start);
    auto vocableLayer = cardBox->add<widget::Layer>(Align::start);
    cardLayer->setName("cardLayer");
    vocableLayer->setName("vocableLayer");
    cardLayer->setExpandType(width_fixed, height_fixed);
    auto treeWalker = co_await asyncTreeWalker->getTreeWalker();
    dataBase = co_await asyncTreeWalker->getDataBase();
    auto wordDB = dataBase->getWordDB();
    auto cardDB = dataBase->getCardDB();
    sr::CardMeta cardMeta;

    signalProceed->set(Proceed::walkTree);
    while (true) {
        Proceed proceed = co_await *signalProceed;

        switch (proceed) {
        case Proceed::walkTree:
            clearStudy(cardLayer, vocableLayer);
            cardMeta = co_await asyncTreeWalker->getNextCardChoice();
            spdlog::info("kocoro, walkTree: loaded CardID {}..", cardMeta.getCardId());
            track = cardDB->getTrackFromCardId(cardMeta.getCardId());
            loadTrack();
            prepareStudy(cardMeta, wordDB, cardLayer, vocableLayer);
            break;
        case Proceed::submit: {
            treeWalker->setEaseForCard(cardMeta.getCardId(), displayVocables->getVocIdEase());
            if (!track.has_value()) {
                mode = Mode::shuffle;
                signalProceed->set(Proceed::walkTree);
            } else {
                signalProceed->set(Proceed::nextTrack);
            }
        } break;
        case Proceed::nextTrack:
            clearStudy(cardLayer, vocableLayer);
            cardMeta = dataBase->getCardMeta(track->getCard());
            spdlog::info("kocoro, nextTrack: loaded CardID {}..", cardMeta.getCardId());
            loadTrack();
            prepareStudy(cardMeta, wordDB, cardLayer, vocableLayer);
            break;
        case Proceed::reload:
            clearStudy(cardLayer, vocableLayer);
            cardMeta = dataBase->getCardMeta(cardMeta.getCardId());
            prepareStudy(cardMeta, wordDB, cardLayer, vocableLayer);
            break;
        case Proceed::annotate: {
            auto cardAnnotationChanged = co_await annotationTask(cardMeta, cardDB, cardLayer);
            if (cardAnnotationChanged) {
                signalProceed->set(Proceed::reload);
            }
        } break;

        default:
            break;
        }
    }

    co_return;
}

auto TabCard::annotationTask(sr::CardMeta& cardMeta,
                             const std::shared_ptr<database::CardDB>& cardDB,
                             std::shared_ptr<widget::Layer> cardLayer) -> kocoro::Task<bool>
{
    auto alternatives = cardDB->getAnnotationAlternativesForCard(cardMeta.getCardId());
    auto tokenText = cardMeta.getStudyTokenText();
    displayAnnotation = std::make_unique<DisplayAnnotation>(cardLayer, overlay, alternatives, std::move(tokenText));
    auto tokenizationChoice = co_await *signalAnnotationDone;
    displayAnnotation.reset();
    if (!tokenizationChoice.empty()) {
        auto tokenizationChoiceDB = dataBase->getTokenizationChoiceDB();
        auto card = cardDB->getCards().at(cardMeta.getCardId());
        tokenizationChoiceDB->insertTokenization(tokenizationChoice, card);
        dataBase->reloadCard(card);

        signalProceed->set(Proceed::reload);
        co_return true;
    }
    co_return false;
}

void TabCard::clearStudy(const std::shared_ptr<widget::Layer>& cardLayer,
                         const std::shared_ptr<widget::Layer>& vocableLayer)
{
    displayText.reset();
    displayVocables.reset();
    cardLayer->clear();
    vocableLayer->clear();
}

void TabCard::prepareStudy(sr::CardMeta& cardMeta,
                           std::shared_ptr<database::WordDB> wordDB,
                           const std::shared_ptr<widget::Layer>& cardLayer,
                           const std::shared_ptr<widget::Layer>& vocableLayer)
{
    auto vocId_ease = cardMeta.getRelevantEase();
    auto tokenText = cardMeta.getStudyTokenText();

    auto orderedVocId_ease = tokenText->setupActiveVocableIds(vocId_ease);
    displayText = std::make_unique<DisplayText>(cardLayer, overlay, std::move(tokenText));
    if (!vocId_ease.empty()) {
        displayVocables = std::make_unique<DisplayVocables>(vocableLayer, wordDB, std::move(orderedVocId_ease));
    }
}

void TabCard::loadTrack()
{
    if (mpvAudio && !mpvAudio->is_paused()) {
        mpvAudio->pause();
    }
    if (mpvVideo && !mpvVideo->is_paused()) {
        mpvVideo->pause();
    }
    if (!track->getMediaFile().has_value()) {
        return;
    }
    if (track->getTrackType() == database::TrackType::audio) {
        mpvAudio->openFile(track->getMediaFile().value());
    }
    if (track->getTrackType() == database::TrackType::video) {
        mpvVideo->openFile(track->getMediaFile().value());
        double start = track->getStartTimeStamp();
        double end = track->getEndTimeStamp();
        mpvVideo->play_fragment(start, end);
        mpvVideo->pause();
    }
}

void TabCard::setupCardWindow(widget::Window& cardWindow)
{
    auto cardBox = cardWindow.add<widget::Box>(Align::start, widget::Orientation::vertical);
    overlay = cardWindow.add<widget::Overlay>(Align::start);
    // video = cardWindow.add<widget::Video>(Align::start, mpvVideo);
    cardBox->setName("cardBox");

    signalCardBox->set(cardBox);

    displayVideo->setUp(cardWindow.getLayer());
}

void TabCard::doCardWindow(widget::Window& cardWindow)
{
    auto droppedWindow = cardWindow.dropWindow();
    if (track && track->getTrackType() == database::TrackType::video) {
        displayVideo->displayOnLayer(cardWindow.getLayer());
    }

    // video->displayTexture();
    // first draw displayVocables to prevent flickering on configuring vocable (vocableOverlay)
    if (displayVocables && revealVocables) {
        displayVocables->draw();
    }
    if (displayAnnotation) {
        displayAnnotation->draw();
        auto choice = displayAnnotation->getChoice();
        if (!choice.empty()) {
            signalAnnotationDone->set(choice);
        }
    } else if (displayText) {
        if (displayText->draw() && displayVocables) {
            displayVocables->reload();
        }
    }
}

void TabCard::setupCtrlWindow(widget::Window& ctrlWindow)
{
    using namespace widget::layout;
    auto& audioCtrlBox = *ctrlWindow.add<widget::Box>(Align::start, widget::Orientation::horizontal);
    auto& videoCtrlBox = *ctrlWindow.add<widget::Box>(Align::start, widget::Orientation::horizontal);
    setupAudioCtrlBox(audioCtrlBox);
    setupVideoCtrlBox(videoCtrlBox);
}

void TabCard::doCtrlWindow(widget::Window& ctrlWindow)
{
    auto droppedWindow = ctrlWindow.dropWindow();
    ctrlWindow.start();
    auto& audioCtrlBox = ctrlWindow.next<widget::Box>();
    auto& videoCtrlBox = ctrlWindow.next<widget::Box>();

    if (mode == Mode::story && track.has_value() && track->getTrackType() == database::TrackType::video) {
        doVideoCtrlBox(videoCtrlBox);
    } else {
        doAudioCtrlBox(audioCtrlBox);
    }
}

void TabCard::setupAudioCtrlBox(widget::Box& ctrlBox)
{
    using namespace widget::layout;
    using context::Image;
    ctrlBox.setName("ctrlBoxAudio");
    ctrlBox.setPadding(4.F);
    ctrlBox.setExpandType(width_expand, height_fixed);

    ctrlBox.add<widget::ImageButton>(Align::start, widget::Images{Image::media_playback_start,
                                                                  Image::media_playback_pause});
    // ctrlBox.add<widget::Separator>(Align::start, 4.F, 0.F);
    ctrlBox.add<widget::MediaSlider>(Align::start);
    ctrlBox.add<widget::Separator>(Align::end, 16.F, 0.F);

    setupCtrlBoxRight(ctrlBox);
}

void TabCard::doAudioCtrlBox(widget::Box& ctrlBox)
{
    ctrlBox.start();
    auto& btnPlay = ctrlBox.next<widget::ImageButton>();
    auto& sliderProgress = ctrlBox.next<widget::MediaSlider>();
    ctrlBox.next<widget::Separator>();

    handlePlayback(btnPlay, sliderProgress);

    doCtrlBoxRight(ctrlBox);
}

void TabCard::setupVideoCtrlBox(widget::Box& ctrlBox)
{
    using namespace widget::layout;
    using context::Image;
    ctrlBox.setName("ctrlBoxVideo");
    ctrlBox.setPadding(4.F);
    ctrlBox.setExpandType(width_expand, height_fixed);

    ctrlBox.add<widget::ImageButton>(Align::start, widget::Images{Image::media_playback_start,
                                                                  Image::media_playback_pause});
    // ctrlBox.add<widget::Separator>(Align::start, 4.F, 0.F);
    ctrlBox.add<widget::MediaSlider>(Align::start);
    ctrlBox.add<widget::Separator>(Align::end, 16.F, 0.F);

    ctrlBox.add<widget::ImageButton>(Align::start, widget::Images{Image::circle_stop,
                                                                  Image::circle_play,
                                                                  Image::circle_fast_forward,
                                                                  Image::circle_forward});
    ctrlBox.add<widget::Separator>(Align::end, 16.F, 0.F);
    ctrlBox.add<widget::ImageButton>(Align::end, Image::sub_cut_prev);
    ctrlBox.add<widget::ImageButton>(Align::end, Image::sub_add_prev);
    ctrlBox.add<widget::ImageButton>(Align::end, Image::sub_add_next);
    ctrlBox.add<widget::ImageButton>(Align::end, Image::sub_cut_next);
    setupCtrlBoxRight(ctrlBox);
}

void TabCard::doVideoCtrlBox(widget::Box& ctrlBox)
{
    ctrlBox.start();
    auto& btnPlay = ctrlBox.next<widget::ImageButton>();
    auto& sliderProgress = ctrlBox.next<widget::MediaSlider>();

    ctrlBox.next<widget::Separator>();
    auto& btnPlayMode = ctrlBox.next<widget::ImageButton>();
    handlePlayMode(btnPlayMode);

    ctrlBox.next<widget::Separator>();
    auto& btnCutPrev = ctrlBox.next<widget::ImageButton>();
    auto& btnAddPrev = ctrlBox.next<widget::ImageButton>();
    auto& btnAddNext = ctrlBox.next<widget::ImageButton>();
    auto& btnCutNext = ctrlBox.next<widget::ImageButton>();
    handleSubAddCut(btnCutPrev,
                    btnAddPrev,
                    btnAddNext,
                    btnCutNext);

    handlePlayback(btnPlay, sliderProgress);

    doCtrlBoxRight(ctrlBox);
}

void TabCard::setupCtrlBoxRight(widget::Box& ctrlBox)
{
    using namespace widget::layout;
    using context::Image;
    auto& layer = *ctrlBox.add<widget::Layer>(Align::end);
    layer.setExpandType(width_fixed, height_adapt);

    layer.add<widget::Button>(Align::center, " reveal vocabulary ")->setExpandType(width_fixed, height_adapt);
    layer.add<widget::Button>(Align::center, " submit choice of ease ")->setExpandType(width_fixed, height_adapt);
    layer.add<widget::Button>(Align::center, " next ")->setExpandType(width_fixed, height_adapt);

    ctrlBox.add<widget::Separator>(Align::end, 16.F, 0.F);
    ctrlBox.add<widget::ToggleButtonGroup>(Align::end, widget::Orientation::horizontal,
                                           std::initializer_list<std::string>{" shuffle mode ", " story mode "})
            ->setExpandType(width_fixed, height_adapt);

    ctrlBox.add<widget::Separator>(Align::end, 16.F, 0.F);
    ctrlBox.add<widget::ImageButton>(Align::end, Image::media_skip_backward);
    ctrlBox.add<widget::ImageButton>(Align::end, Image::media_seek_backward);
    ctrlBox.add<widget::ImageButton>(Align::end, Image::media_seek_forward);
    ctrlBox.add<widget::ImageButton>(Align::end, Image::media_skip_forward);
    ctrlBox.add<widget::Separator>(Align::end, 32.F, 0.F);
    ctrlBox.add<widget::ImageButton>(Align::end, Image::configure_mid);
    ctrlBox.add<widget::Separator>(Align::end, 8.F, 0.F);
    ctrlBox.add<widget::ImageButton>(Align::end, Image::document_save);
}

void TabCard::doCtrlBoxRight(widget::Box& ctrlBox)
{
    auto& layer = ctrlBox.next<widget::Layer>();
    layer.start();

    auto& btnReveal = layer.next<widget::Button>();
    auto& btnSubmit = layer.next<widget::Button>();
    auto& btnNext = layer.next<widget::Button>();
    handleCardSubmission(btnReveal, btnSubmit, btnNext);

    ctrlBox.next<widget::Separator>();
    auto& tbgMode = ctrlBox.next<widget::ToggleButtonGroup>();
    handleMode(tbgMode);

    ctrlBox.next<widget::Separator>();
    auto& btnFirst = ctrlBox.next<widget::ImageButton>();
    auto& btnPrevious = ctrlBox.next<widget::ImageButton>();
    auto& btnFollowing = ctrlBox.next<widget::ImageButton>();
    auto& btnLast = ctrlBox.next<widget::ImageButton>();
    handleNextPrevious(btnFirst, btnPrevious, btnFollowing, btnLast);

    ctrlBox.next<widget::Separator>();
    auto& btnAnnotate = ctrlBox.next<widget::ImageButton>();
    handleAnnotate(btnAnnotate);

    ctrlBox.next<widget::Separator>();
    auto& btnSave = ctrlBox.next<widget::ImageButton>();
    handleDataBaseSave(btnSave);
}

void TabCard::handlePlayback(widget::ImageButton& btnPlay, widget::MediaSlider& sliderProgress)
{
    if (!track.has_value() || !track->getMediaFile().has_value()) {
        return;
    }
    const auto& mpvCurrent = track->getTrackType() == database::TrackType::audio
                                     ? mpvAudio
                                     : mpvVideo;
    double start = track->getStartTimeStamp();
    double end = track->getEndTimeStamp();
    double timePos = std::max(mpvCurrent->getTimePos(), start);

    unsigned playing = mpvCurrent->is_paused() ? 0 : 1;
    auto oldPlaying = playing;
    playing = btnPlay.toggled(oldPlaying);
    if (oldPlaying != playing) {
        if (playing != 0) {
            if (timePos >= end - 0.05) {
                timePos = start;
            }
            mpvCurrent->play_fragment(timePos, end);
        } else {
            mpvCurrent->pause();
        }
    }
    auto oldTimePos = std::exchange(timePos, sliderProgress.slide(start, end, timePos));
    if (oldTimePos != timePos) {
        if (mpvCurrent->is_paused()) {
            mpvCurrent->play_fragment(timePos, end);
        } else {
            mpvCurrent->seek(timePos);
        }
    }
}

void TabCard::handleCardSubmission(widget::Button& btnReveal, widget::Button& btnSubmit, widget::Button& btnNext)
{
    if (displayText == nullptr) {
        return;
    }

    enum class CardSubmission {
        reveal,
        next,
        submit
    };
    CardSubmission cardSubmission{};
    bool btnClicked = false;

    if (displayVocables == nullptr) {
        btnClicked |= btnNext.clicked();
        cardSubmission = CardSubmission::next;
    } else if (revealVocables) {
        btnClicked |= btnSubmit.clicked();
        cardSubmission = CardSubmission::submit;
    } else {
        btnClicked |= btnReveal.clicked();
        cardSubmission = CardSubmission::reveal;
    }
    if (!btnClicked) {
        return;
    }

    if (utl::isEither(cardSubmission, {CardSubmission::submit,
                                       CardSubmission::next})) {
        switch (mode) {
        case Mode::shuffle:
            track = {};
            break;
        case Mode::story:
            track = track->nextTrack();
            break;
        }
    }
    switch (cardSubmission) {
    case CardSubmission::reveal:
        revealVocables = true;
        break;
    case CardSubmission::next:
        revealVocables = false;
        if (track.has_value()) {
            signalProceed->set(Proceed::nextTrack);
        } else {
            signalProceed->set(Proceed::walkTree);
        }
        break;
    case CardSubmission::submit:
        revealVocables = false;
        signalProceed->set(Proceed::submit);
        break;
    }
}

void TabCard::handleMode(widget::ToggleButtonGroup& tbgMode)
{
    if (!track.has_value() || !track->hasNext()) {
        return;
    }

    mode = static_cast<Mode>(tbgMode.Active(static_cast<unsigned>(mode)));
}

void TabCard::handlePlayMode(widget::ImageButton& btnPlayMode)
{
    playMode = btnPlayMode.toggled(playMode);
}

void TabCard::handleSubAddCut(widget::ImageButton& btnCutPrev,
                              widget::ImageButton& btnAddPrev,
                              widget::ImageButton& btnAddNext,
                              widget::ImageButton& btnCutNext)
{
    if (!track.has_value()) {
        return;
    }
    bool cutAddClicked = false;
    btnCutPrev.setSensitive(track->isSeparable());
    btnAddPrev.setSensitive(track->isFrontJoinable());
    btnAddNext.setSensitive(track->isBackJoinable());
    btnCutNext.setSensitive(track->isSeparable());

    if (btnCutPrev.clicked()) {
        cutAddClicked = true;
        track = track->cutFront();
    }
    if (btnAddPrev.clicked()) {
        cutAddClicked = true;
        track = track->joinFront();
    }
    if (btnAddNext.clicked()) {
        cutAddClicked = true;
        track = track->joinBack();
    }
    if (btnCutNext.clicked()) {
        cutAddClicked = true;
        track = track->cutBack();
    }
    if (cutAddClicked) {
        revealVocables = false;
        signalProceed->set(Proceed::nextTrack);
    }
}

void TabCard::handleNextPrevious(widget::ImageButton& btnFirst,
                                 widget::ImageButton& btnPrevious,
                                 widget::ImageButton& btnFollowing,
                                 widget::ImageButton& btnLast)
{
    if (!track.has_value()
        || (!track->hasNext()
            && !track->hasPrevious())) {
        return;
    }

    btnFirst.setSensitive(track->hasPrevious());
    btnPrevious.setSensitive(track->hasPrevious());
    btnFollowing.setSensitive(track->hasNext());
    btnLast.setSensitive(track->hasNext());
    bool nextPreviousClicked = false;
    if (btnFirst.clicked()) {
        nextPreviousClicked = true;
        track = track->trackAt(0);
    }
    if (btnPrevious.clicked()) {
        nextPreviousClicked = true;
        track = track->previousTrack();
    }
    if (btnFollowing.clicked()) {
        nextPreviousClicked = true;
        track = track->nextTrack();
    }
    if (btnLast.clicked()) {
        nextPreviousClicked = true;
        track = track->trackAt(track->numberOfTracks() - 1);
    }
    if (nextPreviousClicked) {
        mode = Mode::story;
        revealVocables = false;
        signalProceed->set(Proceed::nextTrack);
    }
}

void TabCard::handleAnnotate(widget::ImageButton& btnAnnotate)
{
    if (!dataBase /* || !displayText */) {
        return;
    }
    if (btnAnnotate.clicked()) {
        if (btnAnnotate.isChecked()) {
            signalAnnotationDone->set({});
        } else {
            signalProceed->set(Proceed::annotate);
        }
    }
    btnAnnotate.setChecked(displayAnnotation != nullptr);
}

void TabCard::handleDataBaseSave(widget::ImageButton& btnSave)
{
    if (!dataBase) {
        return;
    }
    if (btnSave.clicked()) {
        dataBase->save();
    }
}

} // namespace gui
