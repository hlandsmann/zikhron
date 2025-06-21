#include "TabCard.h"

#include "imgui.h"

#include <DisplayAnnotation.h>
#include <DisplayText.h>
#include <DisplayVideo.h>
#include <DisplayVocables.h>
#include <ImGuiFileDialog/ImGuiFileDialog.h>
#include <VocableOverlay.h>
#include <annotation/Token.h>
#include <context/Fonts.h>
#include <context/imglog.h>
#include <database/CardDB.h>
#include <database/CardPackDB.h>
#include <database/TokenizationChoiceDbChi.h>
#include <database/Track.h>
#include <database/VideoDB.h>
#include <database/VideoSet.h>
#include <database/WordDB.h>
#include <misc/Identifier.h>
#include <misc/Language.h>
#include <misc/TokenizationChoice.h>
#include <multimedia/MpvWrapper.h>
#include <spaced_repetition/AsyncTreeWalker.h>
#include <spaced_repetition/CardMeta.h>
#include <spaced_repetition/ITreeWalker.h>
#include <spdlog/spdlog.h>
#include <utils/Algorithm.h>
#include <widgets/Box.h>
#include <widgets/Button.h>
#include <widgets/ImageButton.h>
#include <widgets/Layer.h>
#include <widgets/MediaSlider.h>
#include <widgets/Overlay.h>
#include <widgets/Separator.h>
#include <widgets/SteppedSlider.h>
#include <widgets/TextTokenSeq.h>
#include <widgets/ToggleButtonGroup.h>
#include <widgets/Video.h>
#include <widgets/Window.h>
#include <widgets/detail/Widget.h>

#include <algorithm>
#include <exception>
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
    , signalLanguage{executor->makePersistentSignal<Language>()}
    , displayVideo{std::move(_displayVideo)}
    , mpvAudio{std::move(_mpvAudio)}
    , mpvVideo{displayVideo->getMpv()}
{
    executor->startCoro(videoPlaybackTask());
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

void TabCard::setLanguage(Language language)
{
    signalLanguage->set(language);
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
    const auto& [_, videoChoice] = videoSet->getChoice();

    track = cardDB->getTrackFromVideo(videoChoice);
    if (track->hasSubtitlePrefix()) {
        track = track->getSubtitlePrefix();
    }

    signalProceed->set(Proceed::nextTrack);
    mode = Mode::story;
}

auto TabCard::videoPlaybackTask() -> kocoro::Task<>
{
    auto localTrack = track;
    while (true) {
        double timePos = co_await mpvVideo->SignalTimePos();
        if (localTrack != track) { // this only happens if next is pressed manually.
            localTrack = track;
            if (localTrack && localTrack->getStartTimeStamp() > timePos) {
                mpvVideo->seek(track->getStartTimeStamp());
                continue;
            }
        }
        if (timePos >= track->getEndTimeStamp()) {
            // track is set here, so set localTrack at the end of switch/case
            auto tmpPlayMode = evaluateTemporaryPlaymode();
            switch (tmpPlayMode) {
            case PlayMode::stop:
                if (track->isSubtitlePrefix()) {
                    track = track->getNonPrefixDefault();
                    signalProceed->set(Proceed::nextTrack);
                } else {
                    // mpvVideo->pause();
                    mpvVideo->setStopMark(track->getEndTimeStamp());
                    // isReviewingSubtitle = false;
                }
                break;
            case PlayMode::play:
                execVideoNext();
                signalProceed->set(Proceed::nextTrack);
                break;
            }
            localTrack = track;
        }
        // if (mode != Mode::story) {
        //     continue;
        // }
        // spdlog::info("vpt,timeps: {}", timePos);
        // if (timePos < track->getStartTimeStamp()) {
        //     mpvVideo->seek(track->getStartTimeStamp());
        // }
    }
    co_return;
}

auto TabCard::feedingTask(std::shared_ptr<sr::AsyncTreeWalker> asyncTreeWalker) -> kocoro::Task<>
{
    using namespace widget::layout;
    auto cardBox = co_await *signalCardBox;
    auto cardLayer = cardBox->add<widget::Layer>(Align::start);
    auto vocableLayer = cardBox->add<widget::Layer>(Align::start);
    auto translationLayer = cardBox->add<widget::Layer>(Align::end);
    auto metaLayer = cardBox->add<widget::Layer>(Align::end);
    cardLayer->setName("cardLayer");
    vocableLayer->setName("vocableLayer");
    cardLayer->setExpandType(width_fixed, height_fixed);
    auto language = co_await *signalLanguage;
    auto treeWalker = co_await asyncTreeWalker->getTreeWalker(language);
    dataBase = co_await asyncTreeWalker->getDataBase(language);
    auto wordDB = dataBase->getWordDB();
    auto cardDB = dataBase->getCardDB();
    sr::CardMeta cardMeta;

    signalProceed->set(Proceed::walkTree);
    while (true) {
        Proceed proceed = co_await *signalProceed;

        switch (proceed) {
        case Proceed::walkTree: {
            clearStudy(cardLayer, vocableLayer, translationLayer);
            cardMeta = co_await asyncTreeWalker->getNextCardChoice(language);
            spdlog::info("kocoro, walkTree: loaded CardID {}..", cardMeta.getCardId());
            cardMeta.getCard()->dumpDebugLog();

            track = cardDB->getTrackFromCardId(cardMeta.getCardId());
            loadTrack();
            prepareStudy(cardMeta, cardLayer, vocableLayer, translationLayer, metaLayer, treeWalker, language);
        } break;
        case Proceed::submit: {
            dataBase->rateCard(cardMeta.getCard(), displayVocables->getRatedVocables());
            if (!track.has_value()) {
                mode = Mode::shuffle;
                signalProceed->set(Proceed::walkTree);
            } else {
                signalProceed->set(Proceed::nextTrack);
            }
        } break;
        case Proceed::nextTrack: {
            clearStudy(cardLayer, vocableLayer, translationLayer);
            cardMeta = dataBase->getCardMeta(track->getCard());
            spdlog::info("kocoro, nextTrack: loaded CardID {}..", cardMeta.getCardId());
            cardMeta.getCard()->dumpDebugLog();
            loadTrack();
            if (!track->isSubtitlePrefix()) {
                prepareStudy(cardMeta, cardLayer, vocableLayer, translationLayer, metaLayer, treeWalker, language);
            }
        } break;
        case Proceed::reload:
            clearStudy(cardLayer, vocableLayer, translationLayer);
            cardMeta = dataBase->getCardMeta(cardMeta.getCard());
            prepareStudy(cardMeta, cardLayer, vocableLayer, translationLayer, metaLayer, treeWalker, language);
            break;
        case Proceed::reloadCardOnly:

        case Proceed::annotate: {
            spdlog::info("co_await annotationTask");
            auto cardAnnotationChanged = co_await annotationTask(cardMeta, cardDB, cardLayer);
            spdlog::info("co_await annotationTask done");
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
    try {
        auto alternatives = cardDB->getAnnotationAlternativesForCard(cardMeta.getCardId());
        auto tokenText = cardMeta.getStudyTokenText();
        displayAnnotation = std::make_unique<DisplayAnnotation>(cardLayer, overlayForAnnotation, alternatives, std::move(tokenText));
        auto tokenizationChoice = co_await *signalAnnotationDone;
        displayAnnotation.reset();
        if (!tokenizationChoice.empty()) {
            auto tokenizationChoiceDB = dataBase->getTokenizationChoiceDB();
            auto tokenizationChoiceDbChi = std::dynamic_pointer_cast<database::TokenizationChoiceDbChi>(tokenizationChoiceDB);
            if (!tokenizationChoiceDbChi) {
                co_return false;
            }
            auto card = cardDB->getCards().at(cardMeta.getCardId());
            tokenizationChoiceDbChi->insertTokenization(tokenizationChoice, card);
            dataBase->reloadCard(card);

            signalProceed->set(Proceed::reload);
            co_return true;
        }
        co_return false;
    } catch (const std::exception& e) {
        spdlog::error("annotation task crash: {}", e.what());
        co_return false;
    }
}

void TabCard::clearStudy(const std::shared_ptr<widget::Layer>& cardLayer,
                         const std::shared_ptr<widget::Layer>& vocableLayer,
                         const std::shared_ptr<widget::Layer>& translationLayer)
{
    displayText.reset();
    displayVocables.reset();
    ttqTranslation.reset();
    cardLayer->clear();
    vocableLayer->clear();
    translationLayer->clear();
}

void TabCard::prepareStudy(sr::CardMeta& cardMeta,
                           const std::shared_ptr<widget::Layer>& cardLayer,
                           const std::shared_ptr<widget::Layer>& vocableLayer,
                           const std::shared_ptr<widget::Layer>& translationLayer,
                           const std::shared_ptr<widget::Layer>& metaLayer,
                           const std::shared_ptr<sr::ITreeWalker>& treeWalker,
                           Language language)
{
    constexpr static widget::TextTokenSeq::Config ttqConfig = {.fontType = context::FontType::chineseSmall,
                                                               .wordPadding = 10.F,
                                                               .border = 8.F};
    auto activeVocableIds = cardMeta.getActiveVocableIds();
    auto tokenText = cardMeta.getStudyTokenText();

    auto activeVocablesColered = tokenText->setupActiveVocableIds(activeVocableIds);
    displayText = std::make_unique<DisplayText>(cardLayer, overlayForVocable, std::move(tokenText), dataBase, language);
    if (!activeVocableIds.empty()) {
        displayVocables = std::make_unique<DisplayVocables>(vocableLayer, dataBase, std::move(activeVocablesColered), language);
    }
    if (auto optTranslation = track->getTranslation(); optTranslation.has_value() && track->getTrackType() == TrackType::audio) {
        auto translation = *optTranslation;
        ttqTranslation = translationLayer->add<widget::TextTokenSeq>(Align::start,
                                                                     annotation::tokenVectorFromString(translation),
                                                                     ttqConfig);
    }
    metaLayer->clear();
    auto metaInformation = fmt::format("Study today: {}, failed: {}, total vocables: {}",
                                       treeWalker->getNumberOfTodayVocables(),
                                       treeWalker->getNumberOfFailedVocables(),
                                       dataBase->getNumberOfEnabledVocables());
    ttqMetaInformation = metaLayer->add<widget::TextTokenSeq>(Align::start, annotation::tokenVectorFromString(metaInformation),
                                                              ttqConfig);
}

void TabCard::loadTrack()
{
    if (mpvAudio && mpvAudio->is_playing()) {
        mpvAudio->pause();
    }
    if (mpvVideo && mpvVideo->is_playing() && mode == Mode::shuffle) {
        mpvVideo->pause();
    }
    if (!track->getMediaFile().has_value()) {
        return;
    }
    if (track->getTrackType() == database::TrackType::audio) {
        mpvAudio->openFile(track->getMediaFile().value());
    }
    if (track->getTrackType() == database::TrackType::video) {
        if (mpvVideo->getMediaFile() != track->getMediaFile()) {
            mpvVideo->openFile(track->getMediaFile().value());
            mpvVideo->setSubtitle(revealTranslation);
        }
        if (mode == Mode::shuffle) {
            double start = track->getStartTimeStamp();
            double end = track->getEndTimeStamp();
            mpvVideo->setFragment(start, end);
        }
    }
}

void TabCard::setupCardWindow(widget::Window& cardWindow)
{
    auto cardBox = cardWindow.add<widget::Box>(Align::start, widget::Orientation::vertical);
    cardBox->setExpandType(ExpandType::width_fixed, ExpandType::height_expand);
    overlayForVocable = cardWindow.add<widget::Overlay>(Align::start);
    overlayForAnnotation = cardWindow.add<widget::Overlay>(Align::start);
    overlayForTranslation = cardWindow.add<widget::Overlay>(Align::start);
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
        if (displayText->draw()) {
            signalProceed->set(Proceed::reload);
        }
    }
    if (ttqTranslation && revealTranslation) {
        ttqTranslation->draw();
    }
    if (ttqMetaInformation) {
        ttqMetaInformation->draw();
    }
}

void TabCard::setupCtrlWindow(widget::Window& ctrlWindow)
{
    using namespace widget::layout;
    auto& ctrlBox = *ctrlWindow.add<widget::Box>(Align::start, widget::Orientation::vertical);
    secondaryCtrlLayer = ctrlBox.add<widget::Layer>(Align::start);
    setupSecondaryCtrl(*secondaryCtrlLayer);

    auto& primaryCtrlLayer = *ctrlBox.add<widget::Layer>(Align::start);

    auto& audioCtrlBox = *primaryCtrlLayer.add<widget::Box>(Align::start, widget::Orientation::horizontal);
    setupAudioCtrlBox(audioCtrlBox);
    auto& videoCtrlBox = *primaryCtrlLayer.add<widget::Box>(Align::start, widget::Orientation::horizontal);
    setupVideoCtrlBox(videoCtrlBox);
}

void TabCard::doCtrlWindow(widget::Window& ctrlWindow)
{
    auto droppedWindow = ctrlWindow.dropWindow();
    ctrlWindow.start();
    auto& ctrlBox = ctrlWindow.next<widget::Box>();
    ctrlBox.start();
    /* secondaryCtrlLayer = */ ctrlBox.next<widget::Layer>();
    doSecondaryCtrl(*secondaryCtrlLayer);

    auto& primaryCtrlLayer = ctrlBox.next<widget::Layer>();
    primaryCtrlLayer.start();
    auto& audioCtrlBox = primaryCtrlLayer.next<widget::Box>();
    auto& videoCtrlBox = primaryCtrlLayer.next<widget::Box>();

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
    ctrlBox.add<widget::MediaSlider>(Align::start)->setUseKeyboard(true);

    ctrlBox.add<widget::Separator>(Align::end, 16.F, 0.F);
    ctrlBox.add<widget::ImageButton>(Align::end, Image::media_skip_backward);
    ctrlBox.add<widget::ImageButton>(Align::end, Image::media_seek_backward);
    ctrlBox.add<widget::ImageButton>(Align::end, Image::media_seek_forward);
    ctrlBox.add<widget::ImageButton>(Align::end, Image::arrow_up);

    ctrlBox.add<widget::Separator>(Align::end, 16.F, 0.F);
    auto& layer = *ctrlBox.add<widget::Layer>(Align::end);
    layer.setExpandType(width_fixed, height_adapt);
    layer.add<widget::Button>(Align::center, " reveal vocabulary ")->setExpandType(width_fixed, height_adapt);
    layer.add<widget::Button>(Align::center, " submit choice of ease ")->setExpandType(width_fixed, height_adapt);
    layer.add<widget::Button>(Align::center, " next ")->setExpandType(width_fixed, height_adapt);

    ctrlBox.add<widget::Separator>(Align::end, 16.F, 0.F);
    ctrlBox.add<widget::ToggleButtonGroup>(Align::end, widget::Orientation::horizontal,
                                           std::initializer_list<std::string>{" shuffle mode ", " story mode "})
            ->setExpandType(width_fixed, height_adapt);

    ctrlBox.add<widget::Separator>(Align::end, 32.F, 0.F);
    ctrlBox.add<widget::ImageButton>(Align::end, Image::translation);
    ctrlBox.add<widget::Separator>(Align::end, 32.F, 0.F);
    ctrlBox.add<widget::ImageButton>(Align::end, Image::time_subtitle);
    ctrlBox.add<widget::Separator>(Align::end, 8.F, 0.F);
    ctrlBox.add<widget::ImageButton>(Align::end, Image::configure_mid);
    ctrlBox.add<widget::Separator>(Align::end, 8.F, 0.F);
    ctrlBox.add<widget::ImageButton>(Align::end, Image::document_save);
}

void TabCard::doAudioCtrlBox(widget::Box& ctrlBox)
{
    ctrlBox.start();
    auto& btnPlay = ctrlBox.next<widget::ImageButton>();
    auto& sliderProgress = ctrlBox.next<widget::MediaSlider>();
    handlePlayback(btnPlay, sliderProgress);

    ctrlBox.next<widget::Separator>();
    auto& btnFirst = ctrlBox.next<widget::ImageButton>();
    auto& btnPrevious = ctrlBox.next<widget::ImageButton>();
    auto& btnNext = ctrlBox.next<widget::ImageButton>();
    handleNextPrevious(btnFirst, btnPrevious, btnNext);
    auto& btnToggleProgress = ctrlBox.next<widget::ImageButton>();
    handleToggleProgress(btnToggleProgress);

    ctrlBox.next<widget::Separator>();
    auto& layer = ctrlBox.next<widget::Layer>();
    layer.start();
    auto& btnRevealCard = layer.next<widget::Button>();
    auto& btnSubmitCard = layer.next<widget::Button>();
    auto& btnNextCard = layer.next<widget::Button>();
    handleCardSubmission(btnRevealCard, btnSubmitCard, btnNextCard);

    ctrlBox.next<widget::Separator>();
    auto& tbgMode = ctrlBox.next<widget::ToggleButtonGroup>();
    handleMode(tbgMode);

    ctrlBox.next<widget::Separator>();
    auto& btnTranslation = ctrlBox.next<widget::ImageButton>();
    handleTranslation(btnTranslation);

    ctrlBox.next<widget::Separator>();
    auto& btnTimeSubtitle = ctrlBox.next<widget::ImageButton>();
    handleToggleTimeDelAdd(btnTimeSubtitle);

    ctrlBox.next<widget::Separator>();
    auto& btnAnnotate = ctrlBox.next<widget::ImageButton>();
    handleAnnotate(btnAnnotate);

    ctrlBox.next<widget::Separator>();
    auto& btnSave = ctrlBox.next<widget::ImageButton>();
    handleDataBaseSave(btnSave);
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
    ctrlBox.add<widget::MediaSlider>(Align::start)->setUseKeyboard(true);
    ctrlBox.add<widget::Separator>(Align::end, 16.F, 0.F);

    ctrlBox.add<widget::ImageButton>(Align::start, widget::Images{
                                                           Image::circle_stop,
                                                           Image::circle_play});
    ctrlBox.add<widget::Separator>(Align::end, 16.F, 0.F);
    ctrlBox.add<widget::ImageButton>(Align::end, Image::sub_cut_prev);
    ctrlBox.add<widget::ImageButton>(Align::end, Image::sub_add_prev);
    ctrlBox.add<widget::ImageButton>(Align::end, Image::sub_add_next);
    ctrlBox.add<widget::ImageButton>(Align::end, Image::sub_cut_next);
    ctrlBox.add<widget::ImageButton>(Align::end, Image::automatic);

    ctrlBox.add<widget::Separator>(Align::end, 16.F, 0.F);
    ctrlBox.add<widget::ImageButton>(Align::end, Image::object_select);
    ctrlBox.add<widget::ImageButton>(Align::end, Image::object_unselect);

    ctrlBox.add<widget::Separator>(Align::end, 16.F, 0.F);
    ctrlBox.add<widget::ImageButton>(Align::end, Image::media_continue);
    ctrlBox.add<widget::ImageButton>(Align::end, Image::media_seek_backward);
    ctrlBox.add<widget::ImageButton>(Align::end, Image::media_seek_forward);
    ctrlBox.add<widget::ImageButton>(Align::end, Image::arrow_up);

    ctrlBox.add<widget::Separator>(Align::end, 16.F, 0.F);
    auto& layer = *ctrlBox.add<widget::Layer>(Align::end);
    layer.setExpandType(width_fixed, height_adapt);
    layer.add<widget::Button>(Align::center, " reveal vocabulary ")->setExpandType(width_fixed, height_adapt);
    layer.add<widget::Button>(Align::center, " submit choice of ease ")->setExpandType(width_fixed, height_adapt);
    layer.add<widget::Button>(Align::center, " next ")->setExpandType(width_fixed, height_adapt);

    ctrlBox.add<widget::Separator>(Align::end, 16.F, 0.F);
    ctrlBox.add<widget::ToggleButtonGroup>(Align::end, widget::Orientation::horizontal,
                                           std::initializer_list<std::string>{" shuffle mode ", " story mode "})
            ->setExpandType(width_fixed, height_adapt);

    ctrlBox.add<widget::Separator>(Align::end, 32.F, 0.F);
    ctrlBox.add<widget::ImageButton>(Align::end, Image::translation);
    ctrlBox.add<widget::Separator>(Align::end, 32.F, 0.F);
    ctrlBox.add<widget::ImageButton>(Align::end, Image::time_subtitle);
    ctrlBox.add<widget::Separator>(Align::end, 8.F, 0.F);
    ctrlBox.add<widget::ImageButton>(Align::end, Image::configure_mid);
    ctrlBox.add<widget::Separator>(Align::end, 8.F, 0.F);
    ctrlBox.add<widget::ImageButton>(Align::end, Image::document_save);
}

void TabCard::doVideoCtrlBox(widget::Box& ctrlBox)
{
    ctrlBox.start();
    auto& btnPlay = ctrlBox.next<widget::ImageButton>();
    auto& sliderProgress = ctrlBox.next<widget::MediaSlider>();
    handlePlayback(btnPlay, sliderProgress);

    ctrlBox.next<widget::Separator>();
    auto& btnPlayMode = ctrlBox.next<widget::ImageButton>();
    handlePlayMode(btnPlayMode);

    ctrlBox.next<widget::Separator>();
    auto& btnCutPrev = ctrlBox.next<widget::ImageButton>();
    auto& btnAddPrev = ctrlBox.next<widget::ImageButton>();
    auto& btnAddNext = ctrlBox.next<widget::ImageButton>();
    auto& btnCutNext = ctrlBox.next<widget::ImageButton>();
    auto& btnAuto = ctrlBox.next<widget::ImageButton>();
    handleSubAddCut(btnCutPrev,
                    btnAddPrev,
                    btnAddNext,
                    btnCutNext,
                    btnAuto);

    ctrlBox.next<widget::Separator>();
    auto& btnSelect = ctrlBox.next<widget::ImageButton>();
    auto& btnUnselect = ctrlBox.next<widget::ImageButton>();
    handleSelection(btnSelect, btnUnselect);

    ctrlBox.next<widget::Separator>();
    auto& btnContinue = ctrlBox.next<widget::ImageButton>();
    auto& btnPrevious = ctrlBox.next<widget::ImageButton>();
    auto& btnNext = ctrlBox.next<widget::ImageButton>();
    handleNextPreviousVideo(btnContinue, btnPrevious, btnNext);
    auto& btnToggleProgress = ctrlBox.next<widget::ImageButton>();
    handleToggleProgress(btnToggleProgress);

    ctrlBox.next<widget::Separator>();
    auto& layer = ctrlBox.next<widget::Layer>();
    layer.start();

    auto& btnRevealCard = layer.next<widget::Button>();
    auto& btnSubmitCard = layer.next<widget::Button>();
    auto& btnNextCard = layer.next<widget::Button>();
    handleCardSubmission(btnRevealCard, btnSubmitCard, btnNextCard);

    ctrlBox.next<widget::Separator>();
    auto& tbgMode = ctrlBox.next<widget::ToggleButtonGroup>();
    handleMode(tbgMode);

    ctrlBox.next<widget::Separator>();
    auto& btnTranslation = ctrlBox.next<widget::ImageButton>();
    handleTranslation(btnTranslation);

    ctrlBox.next<widget::Separator>();
    auto& btnTimeSubtitle = ctrlBox.next<widget::ImageButton>();
    handleToggleTimeDelAdd(btnTimeSubtitle);

    ctrlBox.next<widget::Separator>();
    auto& btnAnnotate = ctrlBox.next<widget::ImageButton>();
    handleAnnotate(btnAnnotate);

    ctrlBox.next<widget::Separator>();
    auto& btnSave = ctrlBox.next<widget::ImageButton>();
    handleDataBaseSave(btnSave);
}

void TabCard::setupSecondaryCtrl(widget::Layer& ctrlLayer)
{
    using namespace widget::layout;

    auto& ctrlBoxTimeSelection = *ctrlLayer.add<widget::Box>(Align::start, widget::Orientation::horizontal);
    setupTimeSelectionCtrl(ctrlBoxTimeSelection);

    auto& ctrlBoxProgress = *ctrlLayer.add<widget::Box>(Align::start, widget::Orientation::horizontal);
    setupProgressCtrl(ctrlBoxProgress);

    ctrlLayer.setHidden(true);
}

void TabCard::doSecondaryCtrl(widget::Layer& ctrlLayer)
{
    if (ctrlLayer.isHidden()) {
        return;
    }
    ctrlLayer.start();
    auto& ctrlBoxTimeSelection = ctrlLayer.next<widget::Box>();
    auto& ctrlBoxProgress = ctrlLayer.next<widget::Box>();

    switch (secondaryCtrlMode) {
    case SecondaryCtrlMode::progress:
        doProgressCtrl(ctrlBoxProgress);
        break;
    case SecondaryCtrlMode::timeDelAdd:
        doTimeSelectionCtrl(ctrlBoxTimeSelection);
        break;
    }
}

void TabCard::setupTimeSelectionCtrl(widget::Box& ctrlBox)
{
    using namespace widget::layout;
    using context::Image;
    ctrlBox.setExpandType(width_expand, height_fixed);
    ctrlBox.setPadding(4.F);
    ctrlBox.setBorder(4.F);
    ctrlBox.add<widget::ImageButton>(Align::end, Image::time_delete_backward);
    ctrlBox.add<widget::ImageButton>(Align::end, Image::time_backward);
    ctrlBox.add<widget::ImageButton>(Align::end, Image::time_forward);
    ctrlBox.add<widget::ImageButton>(Align::end, Image::time_delete_forward);
    ctrlBox.add<widget::Separator>(Align::end, 88.F, 0.F);
}

void TabCard::doTimeSelectionCtrl(widget::Box& ctrlBox)
{
    ctrlBox.start();
    auto& btnTimeDelFront = ctrlBox.next<widget::ImageButton>();
    auto& btnTimeAddFront = ctrlBox.next<widget::ImageButton>();
    auto& btnTimeAddBack = ctrlBox.next<widget::ImageButton>();
    auto& btnTimeDelBack = ctrlBox.next<widget::ImageButton>();
    handleTimeDelAdd(btnTimeDelFront,
                     btnTimeAddFront,
                     btnTimeAddBack,
                     btnTimeDelBack);
}

void TabCard::setupProgressCtrl(widget::Box& ctrlBox)
{
    using namespace widget::layout;
    using context::Image;
    ctrlBox.setExpandType(width_expand, height_fixed);
    ctrlBox.setPadding(4.F);
    ctrlBox.setBorder(4.F);
    ctrlBox.add<widget::SteppedSlider>(Align::start);
}

void TabCard::doProgressCtrl(widget::Box& ctrlBox)
{
    if (!track
        || (!track->hasNext()
            && !track->hasPrevious())) {
        return;
    }
    ctrlBox.start();
    auto& steppedSlider = ctrlBox.next<widget::SteppedSlider>();

    auto position = track->getPosition();
    auto trackNumber = steppedSlider.slide(track->numberOfTracks() - 1, position);
    if (trackNumber != position) {
        track = track->trackAt(trackNumber);
        revealVocables = false;
        signalProceed->set(Proceed::nextTrack);
        if (track->getMediaFile()) {
            const auto& mpvCurrent = track->getTrackType() == database::TrackType::audio
                                             ? mpvAudio
                                             : mpvVideo;
            mpvCurrent->pause();
            mpvCurrent->seek(track->getStartTimeStamp());
        }
    }
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

    auto playing = mpvCurrent->is_playing();
    auto oldPlaying = playing;
    playing = btnPlay.toggled(oldPlaying);

    if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_R)) {
        isReviewingSubtitle = true;
        mpvCurrent->setFragment(start, end);
        mpvCurrent->play();
        return;
    }
    if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_P)) {
        playing = !playing;
        isReviewingSubtitle = false;
    }
    if (oldPlaying != playing) {
        if (playing) {
            if (timePos >= end - 0.05) {
                if (!isReviewingSubtitle && playMode == PlayMode::play) {
                    mpvCurrent->setStopMark(0);
                    mpvCurrent->play();
                    return;
                }
                timePos = start;
            }
            if (mode == Mode::story && track->getTrackType() == database::TrackType::video) {
                mpvCurrent->playFrom(timePos);
            } else {
                mpvCurrent->setFragment(timePos, end);
                mpvCurrent->play();
            }
        } else {
            mpvCurrent->pause();
        }
    }
    auto oldTimePos = std::exchange(timePos, sliderProgress.slide(start, end, timePos));
    if (oldTimePos != timePos) {
        isReviewingSubtitle = true;
        mpvCurrent->setFragment(timePos, end);
        mpvCurrent->play();
        // if (!mpvCurrent->is_playing()) {
        //     mpvCurrent->setFragment(timePos, end);
        // } else {
        //     mpvCurrent->seek(timePos);
        // }
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
        if (track && track->hasNext()) {
            btnClicked |= btnNext.clicked();
            btnClicked |= ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_Space);
        }
        cardSubmission = CardSubmission::next;
    } else if (revealVocables) {
        btnClicked |= btnSubmit.clicked();
        btnClicked |= ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_S);
        cardSubmission = CardSubmission::submit;
    } else {
        btnClicked |= btnReveal.clicked();
        btnClicked |= ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_Space);
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
            if (track->getTrackType() == TrackType::video) {
                execVideoNext();
            } else {
                track = track->nextTrack();
            }
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
    auto oldMode = mode;
    mode = static_cast<Mode>(tbgMode.Active(static_cast<unsigned>(mode)));
    if (oldMode != mode && mode == Mode::shuffle) {
        auto card = track->getCard();
        auto cardId = card->getCardId();
        if (!dataBase->cardExists(cardId)) {
            signalProceed->set(Proceed::walkTree);
        }
    }
}

void TabCard::handlePlayMode(widget::ImageButton& btnPlayMode)
{
    auto oldPlayMode = std::exchange(playMode, btnPlayMode.toggled(playMode));
    if (oldPlayMode == playMode) {
        return;
    }
    const auto& mpvCurrent = track->getTrackType() == database::TrackType::audio
                                     ? mpvAudio
                                     : mpvVideo;
    switch (playMode) {
    case PlayMode::stop:
        mpvCurrent->setStopMark(track->getEndTimeStamp());
        break;
    case PlayMode::play:
        mpvCurrent->setStopMark(0);
        break;
    }
}

void TabCard::handleSubAddCut(widget::ImageButton& btnCutPrev,
                              widget::ImageButton& btnAddPrev,
                              widget::ImageButton& btnAddNext,
                              widget::ImageButton& btnCutNext,
                              widget::ImageButton& btnAuto)
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
    if (btnAuto.clicked()) {
        cutAddClicked = true;
        track = track->autoJoin();
    }
    if (cutAddClicked) {
        revealVocables = false;
        signalProceed->set(Proceed::nextTrack);
        auto card = track->getCard();
        auto cardId = card->getCardId();
        if (dataBase->cardExists(cardId)) {
            dataBase->removeCard(cardId);
            dataBase->addCard(card);
        }
        dataBase->cleanupCards();
    }
}

void TabCard::handleSelection(widget::ImageButton& btnSelect,
                              widget::ImageButton& btnUnselect)
{
    if (!track) {
        return;
    }
    auto card = track->getCard();
    auto cardId = card->getCardId();
    btnSelect.setSensitive(!dataBase->cardExists(cardId));
    btnUnselect.setSensitive(dataBase->cardExists(cardId));
    if (btnSelect.clicked() || ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_A)) {
        dataBase->addCard(card);
        signalProceed->set(Proceed::reload);
    }
    if (btnUnselect.clicked() || ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_X)) {
        dataBase->removeCard(cardId);
        signalProceed->set(Proceed::reload);
    }
}

void TabCard::handleNextPreviousVideo(widget::ImageButton& btnContinue,
                                      widget::ImageButton& btnPrevious,
                                      widget::ImageButton& btnNext)
{
    if (!track
        || (!track->hasNext()
            && !track->hasPrevious())) {
        return;
    }

    btnPrevious.setSensitive(track->hasPrevious() || track->hasSubtitlePrefix());
    btnNext.setSensitive(track->hasNext());
    bool nextPreviousClicked = false;
    if (btnContinue.clicked()) {
        nextPreviousClicked = true;
        track = track->continueTrack();
    }

    if (btnPrevious.clicked() || ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_B)) {
        nextPreviousClicked = true;
        if (track->hasPrevious()) {
            track = track->previousTrack();
        } else {
            track = track->getSubtitlePrefix();
        }
        isReviewingSubtitle = true;
    }
    if (btnNext.clicked() || ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_N)) {
        isReviewingSubtitle = false;
        nextPreviousClicked = true;
        execVideoNext();
    }

    if (nextPreviousClicked) {
        revealVocables = false;
        signalProceed->set(Proceed::nextTrack);
        mpvVideo->seek(track->getStartTimeStamp());
    }
}

void TabCard::handleNextPrevious(widget::ImageButton& btnFirst,
                                 widget::ImageButton& btnPrevious,
                                 widget::ImageButton& btnNext)
{
    if (!track
        || (!track->hasNext()
            && !track->hasPrevious())) {
        return;
    }

    btnFirst.setSensitive(track->hasPrevious());
    btnPrevious.setSensitive(track->hasPrevious());
    btnNext.setSensitive(track->hasNext());
    bool nextPreviousClicked = false;
    if (btnFirst.clicked()) {
        nextPreviousClicked = true;
        track = track->trackAt(0);
    }
    if (btnPrevious.clicked()) {
        nextPreviousClicked = true;
        track = track->previousTrack();
    }
    if (btnNext.clicked()) {
        nextPreviousClicked = true;
        track = track->nextTrack();
    }
    if (nextPreviousClicked) {
        revealVocables = false;
        signalProceed->set(Proceed::nextTrack);
        const auto& mpvCurrent = track->getTrackType() == database::TrackType::audio
                                         ? mpvAudio
                                         : mpvVideo;
        mpvCurrent->seek(track->getStartTimeStamp());
    }
}

void TabCard::handleToggleProgress(widget::ImageButton& btnToggleProgress)
{
    if (!track
        || (!track->hasNext()
            && !track->hasPrevious())) {
        return;
    }

    btnToggleProgress.setChecked(!secondaryCtrlLayer->isHidden()
                                 && secondaryCtrlMode == SecondaryCtrlMode::progress);
    if (btnToggleProgress.clicked()) {
        secondaryCtrlLayer->setHidden(!secondaryCtrlLayer->isHidden()
                                      && secondaryCtrlMode == SecondaryCtrlMode::progress);
        secondaryCtrlMode = SecondaryCtrlMode::progress;
    }
}

void TabCard::handleToggleTimeDelAdd(widget::ImageButton& btnTimeSubtitle)
{
    if (track && (track->getTrackType() != TrackType::video || track->isSubtitlePrefix())) {
        if (secondaryCtrlMode == SecondaryCtrlMode::timeDelAdd && !secondaryCtrlLayer->isHidden()) {
            secondaryCtrlLayer->setHidden(true);
        }
        return;
    }
    btnTimeSubtitle.setChecked(!secondaryCtrlLayer->isHidden()
                               && secondaryCtrlMode == SecondaryCtrlMode::timeDelAdd);
    if (btnTimeSubtitle.clicked()) {
        secondaryCtrlLayer->setHidden(!secondaryCtrlLayer->isHidden()
                                      && secondaryCtrlMode == SecondaryCtrlMode::timeDelAdd);
        secondaryCtrlMode = SecondaryCtrlMode::timeDelAdd;
    }
}

void TabCard::handleTimeDelAdd(widget::ImageButton& btnTimeDelFront,
                               widget::ImageButton& btnTimeAddFront,
                               widget::ImageButton& btnTimeAddBack,
                               widget::ImageButton& btnTimeDelBack)
{
    if (!track || track->getTrackType() != TrackType::video) {
        return;
    }
    btnTimeDelFront.setSensitive(track->getTimeExtraFront() > 0.);
    btnTimeDelBack.setSensitive(track->getTimeExtraBack() > 0.);

    bool frontClicked = false;
    bool backClicked = false;

    if (btnTimeDelFront.clicked()) {
        track = track->timeDelFront();
        frontClicked = true;
    }
    if (btnTimeAddFront.clicked()) {
        track = track->timeAddFront();
        frontClicked = true;
    }
    if (btnTimeAddBack.clicked()) {
        track = track->timeAddBack();
        backClicked = true;
    }
    if (btnTimeDelBack.clicked()) {
        track = track->timeDelBack();
        backClicked = true;
    }

    if (frontClicked) {
        double start = track->getStartTimeStamp();
        double end = std::min(track->getEndTimeStamp(), start + 0.5);
        mpvVideo->setFragment(start, end);
        mpvVideo->play();
        isReviewingSubtitle = true;
    }
    if (backClicked) {
        double end = track->getEndTimeStamp();
        double start = std::max(track->getStartTimeStamp(), end - 0.5);
        mpvVideo->setFragment(start, end);
        mpvVideo->play();
        isReviewingSubtitle = true;
    }
}

void TabCard::handleTranslation(widget::ImageButton& btnTranslation)
{
    if (!track) {
        return;
    }
    if (track->getTrackType() == TrackType::audio && !ttqTranslation) {
        return;
    }
    btnTranslation.setChecked(revealTranslation);
    if (btnTranslation.clicked() || ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_T)) {
        revealTranslation = !btnTranslation.isChecked();
        if (track && track->getTrackType() == TrackType::video) {
            mpvVideo->setSubtitle(revealTranslation);
            if (!mpvVideo->is_playing()) {
                mpvVideo->seek(mpvVideo->getTimePos() + 0.1);
                mpvVideo->seek(mpvVideo->getTimePos());
            }
        }
    }
}

void TabCard::handleAnnotate(widget::ImageButton& btnAnnotate)
{
    if (!dataBase || !displayText || !dataBase->cardExists(track->getCard()->getCardId())) {
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

auto TabCard::evaluateTemporaryPlaymode() const -> PlayMode
{
    if (playMode == PlayMode::play
        && (mode == Mode::shuffle
            || displayAnnotation
            || (displayText && displayText->vocableOverlayIsActive())
            || isReviewingSubtitle
            || revealVocables)) {
        return PlayMode::stop;
    }
    return playMode;
}

void TabCard::execVideoNext()
{
    if (!track->hasNext()) {
        return;
    }
    switch (playMode) {
    case PlayMode::stop: {
        double endTimeStamp{};
        if (track->isSubtitlePrefix()) {
            track = track->getNonPrefixDefault();
            endTimeStamp = track->getEndTimeStamp();
        } else {
            track = track->nextTrack();
            endTimeStamp = track->getEndTimeStamp();
            if (track->hasSubtitlePrefix()) {
                track = track->getSubtitlePrefix();
            }
        }

        mpvVideo->setFragment(track->getStartTimeStamp(), endTimeStamp);
        mpvVideo->play();
    } break;
    case PlayMode::play: {
        if (track->isSubtitlePrefix()) {
            track = track->getNonPrefixDefault();
        } else {
            track = track->nextTrack();
            if (track->hasSubtitlePrefix()) {
                track = track->getSubtitlePrefix();
            }
        }

        mpvVideo->setFragment(track->getStartTimeStamp(), 0);
        mpvVideo->play();
    } break;
    }
}

} // namespace gui
