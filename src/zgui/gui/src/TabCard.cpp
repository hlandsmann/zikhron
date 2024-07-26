#include "TabCard.h"

#include <DisplayAnnotation.h>
#include <DisplayText.h>
#include <DisplayVideo.h>
#include <DisplayVocables.h>
#include <ImGuiFileDialog/ImGuiFileDialog.h>
#include <VocableOverlay.h>
#include <context/imglog.h>
#include <database/CardPackDB.h>
#include <database/VideoPack.h>
#include <misc/Identifier.h>
#include <misc/TokenizationChoice.h>
#include <multimedia/MpvWrapper.h>
#include <spaced_repetition/AsyncTreeWalker.h>
#include <spaced_repetition/CardMeta.h>
#include <spdlog/spdlog.h>
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
#include <cstddef>
#include <filesystem>
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

TabCard::CardAudioInfo::CardAudioInfo(CardId cardId, const database::CardPackDB& cardPackDB)
    : cardAudio{cardPackDB.getCardAtCardId(cardId)}
    , cardPack{cardPackDB.getCardPackForCardId(cardAudio.card->getCardId())}
{}

auto TabCard::CardAudioInfo::firstId() const -> std::optional<CardId>
{
    auto firstCardId = cardPack->getFirstCard().card->getCardId();
    if (firstCardId != cardAudio.card->getCardId()) {
        return {firstCardId};
    }
    return {};
}

auto TabCard::CardAudioInfo::previousId() const -> std::optional<CardId>
{
    auto previousCard = cardPack->getPreviousCard(cardAudio.card);
    if (!previousCard.has_value()) {
        return {};
    }
    return previousCard->card->getCardId();
}

auto TabCard::CardAudioInfo::nextId() const -> std::optional<CardId>
{
    auto nextCard = cardPack->getNextCard(cardAudio.card);
    if (!nextCard.has_value()) {
        return {};
    }
    return nextCard->card->getCardId();
}

auto TabCard::CardAudioInfo::lastId() const -> std::optional<CardId>
{
    auto lastCardId = cardPack->getLastCard().card->getCardId();
    if (lastCardId != cardAudio.card->getCardId()) {
        return {lastCardId};
    }
    return {};
}

auto TabCard::CardAudioInfo::getAudio() const -> std::optional<std::filesystem::path>
{
    if (cardAudio.audioFile.empty()) {
        return {};
    }
    return {cardAudio.audioFile};
}

auto TabCard::CardAudioInfo::getStartTime() const -> double
{
    return cardAudio.start;
}

auto TabCard::CardAudioInfo::getEndTime() const -> double
{
    return cardAudio.end;
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

void TabCard::slot_playVideoPack(database::VideoPackPtr videoPack)
{
    mpvVideo->openFile(videoPack->getVideo()->getVideoFile());
    mpvVideo->play();
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
    auto cardDB = dataBase->getCardPackDB();

    sr::CardMeta cardMeta;
    Proceed proceed = Proceed::submit_walkTree;
    while (true) {
        if (displayVocables
            && (proceed == Proceed::submit_walkTree
                || proceed == Proceed::submit_next)) {
            treeWalker->setEaseLastCard(displayVocables->getVocIdEase());
        }
        displayText.reset();
        displayVocables.reset();
        cardLayer->clear();
        vocableLayer->clear();

        bool cardIsValid = true;
        switch (proceed) {
        case Proceed::submit_walkTree:
            cardMeta = co_await asyncTreeWalker->getNextCardChoice();
            break;
        case Proceed::first:
            cardMeta = co_await asyncTreeWalker->getNextCardChoice(cardAudioInfo->firstId());
            break;
        case Proceed::previous:
            cardMeta = co_await asyncTreeWalker->getNextCardChoice(cardAudioInfo->previousId());
            break;
        case Proceed::next:
        case Proceed::submit_next:
            if (!cardAudioInfo->nextId().has_value()) {
                mode = Mode::shuffle;
            }
            cardMeta = co_await asyncTreeWalker->getNextCardChoice(cardAudioInfo->nextId());
            break;
        case Proceed::last:
            cardMeta = co_await asyncTreeWalker->getNextCardChoice(cardAudioInfo->lastId());
            break;
        case Proceed::reload:
            cardMeta = co_await asyncTreeWalker->getNextCardChoice({cardMeta.Id()});
            break;
        case Proceed::annotate: {
            cardIsValid = !co_await annotationTask(cardMeta, cardDB, cardLayer);
        } break;

        default:
            break;
        }
        if (mpvAudio && !mpvAudio->is_paused()) {
            mpvAudio->pause();
        }
        if (!cardIsValid) {
            proceed = co_await *signalProceed;
            continue;
        }
        spdlog::info("kocoro, loading CardID {}..", cardMeta.Id());
        cardAudioInfo = std::make_unique<CardAudioInfo>(cardMeta.Id(), *dataBase->getCardPackDB());
        if (cardAudioInfo->getAudio().has_value()) {
            mpvAudio->openFile(cardAudioInfo->getAudio().value());
        }
        auto vocId_ease = cardMeta.getRelevantEase();
        auto tokenText = cardMeta.getStudyTokenText();

        auto orderedVocId_ease = tokenText->setupActiveVocableIds(vocId_ease);
        displayText = std::make_unique<DisplayText>(cardLayer, overlay, std::move(tokenText));
        if (!vocId_ease.empty()) {
            displayVocables = std::make_unique<DisplayVocables>(vocableLayer, wordDB, std::move(orderedVocId_ease));
        }

        proceed = co_await *signalProceed;
    }

    co_return;
}

auto TabCard::annotationTask(sr::CardMeta& cardMeta,
                             const std::shared_ptr<database::CardPackDB>& cardDB,
                             std::shared_ptr<widget::Layer> cardLayer) -> kocoro::Task<bool>
{
    auto alternatives = cardDB->getAnnotationAlternativesForCard(cardMeta.Id());
    auto tokenText = cardMeta.getStudyTokenText();
    displayAnnotation = std::make_unique<DisplayAnnotation>(cardLayer, overlay, alternatives, std::move(tokenText));
    auto tokenizationChoice = co_await *signalAnnotationDone;
    displayAnnotation.reset();
    if (!tokenizationChoice.empty()) {
        auto tokenizationChoiceDB = dataBase->getTokenizationChoiceDB();
        auto card = cardDB->getCardAtCardId(cardMeta.Id()).card;
        tokenizationChoiceDB->insertTokenization(tokenizationChoice, card);
        dataBase->reloadCard(card);

        signalProceed->set(Proceed::reload);
        co_return true;
    }
    co_return false;
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
    displayVideo->displayOnLayer(cardWindow.getLayer());

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
    using context::Image;
    using namespace widget::layout;
    using namespace widget::layout;
    auto& ctrlBox = *ctrlWindow.add<widget::Box>(Align::start, widget::Orientation::horizontal);
    ctrlBox.setName("ctrlBox");
    ctrlBox.setPadding(4.F);
    ctrlBox.setExpandType(width_expand, height_fixed);

    ctrlBox.add<widget::ImageButton>(Align::start, Image::media_playback_start, Image::media_playback_pause);
    // ctrlBox.add<widget::Separator>(Align::start, 4.F, 0.F);
    ctrlBox.add<widget::MediaSlider>(Align::start);
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

void TabCard::doCtrlWindow(widget::Window& ctrlWindow)
{
    auto droppedWindow = ctrlWindow.dropWindow();
    ctrlWindow.start();
    auto& box = ctrlWindow.next<widget::Box>();
    box.start();
    auto& btnPlay = box.next<widget::ImageButton>();
    auto& sliderProgress = box.next<widget::MediaSlider>();
    box.next<widget::Separator>();
    auto& layer = box.next<widget::Layer>();
    layer.start();
    auto& btnReveal = layer.next<widget::Button>();
    auto& btnSubmit = layer.next<widget::Button>();
    auto& btnNext = layer.next<widget::Button>();

    box.next<widget::Separator>();
    auto& tbgMode = box.next<widget::ToggleButtonGroup>();
    box.next<widget::Separator>();
    auto& btnFirst = box.next<widget::ImageButton>();
    auto& btnPrevious = box.next<widget::ImageButton>();
    auto& btnFollowing = box.next<widget::ImageButton>();
    auto& btnLast = box.next<widget::ImageButton>();
    box.next<widget::Separator>();
    auto& btnAnnotate = box.next<widget::ImageButton>();
    box.next<widget::Separator>();
    auto& btnSave = box.next<widget::ImageButton>();

    handlePlayback(btnPlay, sliderProgress);
    handleCardSubmission(btnReveal, btnSubmit, btnNext);
    handleMode(tbgMode);
    handleNextPrevious(btnFirst, btnPrevious, btnFollowing, btnLast);
    handleAnnotate(btnAnnotate);
    handleDataBaseSave(btnSave);
}

void TabCard::handlePlayback(widget::ImageButton& btnPlay, widget::MediaSlider& sliderProgress)
{
    if (!cardAudioInfo || !cardAudioInfo->getAudio().has_value()) {
        return;
    }
    double start = cardAudioInfo->getStartTime();
    double end = cardAudioInfo->getEndTime();
    double timePos = std::max(mpvAudio->getTimePos(), start);

    bool playing = !mpvAudio->is_paused();
    btnPlay.setOpen(playing);
    bool oldPlaying = std::exchange(playing, btnPlay.isOpen());
    if (oldPlaying != playing) {
        if (playing) {
            if (timePos >= end - 0.05) {
                timePos = start;
            }
            mpvAudio->play_fragment(timePos, end);
        } else {
            mpvAudio->pause();
        }
    }
    auto oldTimePos = std::exchange(timePos, sliderProgress.slide(start, end, timePos));
    if (oldTimePos != timePos) {
        if (mpvAudio->is_paused()) {
            mpvAudio->play_fragment(timePos, end);
        } else {
            mpvAudio->seek(timePos);
        }
    }
}

void TabCard::handleCardSubmission(widget::Button& btnReveal, widget::Button& btnSubmit, widget::Button& btnNext)
{
    if (displayText == nullptr) {
        return;
    }
    bool submitOrNextClicked = false;

    if (displayVocables == nullptr) {
        submitOrNextClicked |= btnNext.clicked();
    } else if (revealVocables) {
        submitOrNextClicked |= btnSubmit.clicked();
    } else {
        if (btnReveal.clicked()) {
            revealVocables = true;
        }
    }
    if (submitOrNextClicked) {
        revealVocables = false;
        switch (mode) {
        case Mode::shuffle:
            signalProceed->set(Proceed::submit_walkTree);
            break;
        case Mode::story:
            signalProceed->set(Proceed::submit_next);
            break;
        }
    }
}

void TabCard::handleMode(widget::ToggleButtonGroup& tbgMode)
{
    if (!cardAudioInfo || !cardAudioInfo->nextId().has_value()) {
        return;
    }
    mode = static_cast<Mode>(tbgMode.Active(static_cast<unsigned>(mode)));
}

void TabCard::handleNextPrevious(widget::ImageButton& btnFirst,
                                 widget::ImageButton& btnPrevious,
                                 widget::ImageButton& btnFollowing,
                                 widget::ImageButton& btnLast)
{
    if (!cardAudioInfo
        || (!cardAudioInfo->firstId().has_value()
            && !cardAudioInfo->previousId().has_value()
            && !cardAudioInfo->nextId().has_value()
            && !cardAudioInfo->lastId().has_value())) {
        return;
    }

    // if (!cardAudioInfo.nextId.has_value()) {
    //     mode = Mode::shuffle;
    // }

    btnFirst.setSensitive(cardAudioInfo->firstId().has_value());
    btnPrevious.setSensitive(cardAudioInfo->previousId().has_value());
    btnFollowing.setSensitive(cardAudioInfo->nextId().has_value());
    btnLast.setSensitive(cardAudioInfo->lastId().has_value());
    if (btnFirst.clicked()) {
        mode = Mode::story;
        revealVocables = false;
        signalProceed->set(Proceed::first);
    }
    if (btnPrevious.clicked()) {
        mode = Mode::story;
        revealVocables = false;
        signalProceed->set(Proceed::previous);
    }
    if (btnFollowing.clicked()) {
        mode = Mode::story;
        revealVocables = false;
        signalProceed->set(Proceed::next);
    }
    if (btnLast.clicked()) {
        revealVocables = false;
        signalProceed->set(Proceed::last);
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

    // // open Dialog Simple
    // if (btnSave.clicked()) {
    //     IGFD::FileDialogConfig config;
    //     config.path = ".";
    //     config.flags = ImGuiFileDialogFlags_Modal;
    //     ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".cpp,.h,.hpp", config);
    // }
    //
    // // display
    // if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) {
    //     // action if OK
    //     if (ImGuiFileDialog::Instance()->IsOk()) {
    //         std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
    //         std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
    //         spdlog::warn("open: {}, {}", filePath, filePathName);
    //         // action
    //     }
    //
    //     // close
    //     ImGuiFileDialog::Instance()->Close();
    // }
}

} // namespace gui
