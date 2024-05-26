#include "TabCard.h"

#include <DisplayText.h>
#include <DisplayVocables.h>
#include <VocableOverlay.h>
#include <context/imglog.h>
#include <misc/Identifier.h>
#include <multimedia/CardAudioGroup.h>
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
#include <widgets/Window.h>
#include <widgets/detail/Widget.h>

#include <cstddef>
#include <initializer_list>
#include <kocoro/kocoro.hpp>
#include <memory>
#include <string>
#include <utility>

namespace gui {

TabCard::TabCard(std::shared_ptr<kocoro::SynchronousExecutor> _synchronousExecutor,
                 std::shared_ptr<sr::AsyncTreeWalker> _asyncTreeWalker)
    : executor{std::move(_synchronousExecutor)}
    , signalProceed{executor->makeVolatileSignal<Proceed>()}
    , signalCardBox{executor->makePersistentSignal<BoxPtr>()}
{
    executor->startCoro(feedingTask(std::move(_asyncTreeWalker)));
}

TabCard::CardAudioInfo::CardAudioInfo(CardId cardId, const CardAudioGroupDB& cagDB)
    : firstId{cagDB.skipBackward(cardId)}
    , previousId{cagDB.seekBackward(cardId)}
    , nextId{cagDB.seekForward(cardId)}
    , lastId{cagDB.skipForward(cardId)}
    , audioFragment{cagDB.get_studyAudioFragment(cardId)}
{}

void TabCard::setUp(std::shared_ptr<widget::Layer> layer)
{
    using namespace widget::layout;
    auto box = layer->add<widget::Box>(Align::start, widget::Orientation::vertical);
    box->setName("DisplayCard_box");
    boxId = box->getWidgetId();

    auto& cardWindow = *box->add<widget::Window>(Align::start, width_expand, height_expand, "card_text");
    auto& ctrlWindow = *box->add<widget::Window>(Align::end, ExpandType::width_expand, ExpandType::height_fixed, "card_ctrl");
    setupCardWindow(cardWindow);
    setupCtrlWindow(ctrlWindow);
}

void TabCard::displayOnLayer(widget::Layer& layer)
{
    auto box = layer.getWidget<widget::Box>(boxId);
    box.start();
    doCardWindow(box.next<widget::Window>());
    doCtrlWindow(box.next<widget::Window>());
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

        switch (proceed) {
        case Proceed::submit_walkTree:
            cardMeta = co_await asyncTreeWalker->getNextCardChoice();
            break;
        case Proceed::first:
            cardMeta = co_await asyncTreeWalker->getNextCardChoice(cardAudioInfo.firstId);
            break;
        case Proceed::previous:
            cardMeta = co_await asyncTreeWalker->getNextCardChoice(cardAudioInfo.previousId);
            break;
        case Proceed::next:
        case Proceed::submit_next:
            cardMeta = co_await asyncTreeWalker->getNextCardChoice(cardAudioInfo.nextId);
            break;
        case Proceed::last:
            cardMeta = co_await asyncTreeWalker->getNextCardChoice(cardAudioInfo.lastId);
            break;
        default:
            break;
        }
        cardAudioInfo = CardAudioInfo{cardMeta.Id(), *dataBase->getGroupDB()};
        auto vocId_ease = cardMeta.getRelevantEase();
        auto tokenText = cardMeta.getStudyTokenText();

        auto orderedVocId_ease = tokenText->setupActiveVocableIds(vocId_ease);
        displayText = std::make_unique<DisplayText>(cardLayer, std::move(tokenText));
        if (!vocId_ease.empty()) {
            displayVocables = std::make_unique<DisplayVocables>(vocableLayer, wordDB, std::move(orderedVocId_ease));
        }

        proceed = co_await *signalProceed;
        // signalVocIdEase
        // auto& signalVoIdEase = *signalProceed;
        // const auto& vocIdEase = co_await signalVoIdEase;
    }

    co_return;
}

void TabCard::setupCardWindow(widget::Window& cardWindow)
{
    auto cardBox = cardWindow.add<widget::Box>(Align::start, widget::Orientation::vertical);
    overlay = cardWindow.add<widget::Overlay>(Align::start, VocableOverlay::maxWidth);
    cardBox->setName("cardBox");

    signalCardBox->set(cardBox);
}

void TabCard::doCardWindow(widget::Window& cardWindow)
{
    auto droppedWindow = cardWindow.dropWindow();
    if (displayText) {
        auto textToken = displayText->draw();
        if (textToken.has_value()) {
            vocableOverlay = std::make_unique<VocableOverlay>(overlay, textToken.value());
            const auto& annotationToken = textToken.value()->getToken();
            const auto& rect = textToken.value()->getPositionRect();
            spdlog::info("Clicked token: {}, x: {}, y: {}, h: {}", annotationToken.getValue(), rect.x, rect.y, rect.height);
        }
    }
    if (displayVocables && revealVocables) {
        displayVocables->draw();
    }
    if (vocableOverlay) {
        vocableOverlay->draw();
        if (vocableOverlay->wasConfigured() && displayVocables) {
            displayVocables->reload();
        }
        if (vocableOverlay->shouldClose()) {
            vocableOverlay = nullptr;
        }
    }
    // auto& cardLayer = cardWindow.next<widget::Layer>();
    // cardLayer.start();
    // if (!cardLayer.isLast()) {
    //     cardLayer.next<widget::TextTokenSeq>().draw();
    // }
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
    ctrlBox.add<widget::Separator>(Align::end, 16.F, 0.F);
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
    auto& btnSave = box.next<widget::ImageButton>();

    handlePlayback(btnPlay, sliderProgress);
    handleCardSubmission(btnReveal, btnSubmit, btnNext);
    handleMode(tbgMode);
    handleNextPrevious(btnFirst, btnPrevious, btnFollowing, btnLast);
    handleDataBaseSave(btnSave);
}

void TabCard::handlePlayback(widget::ImageButton& btnPlay, widget::MediaSlider& sliderProgress)
{
    if (!cardAudioInfo.audioFragment.has_value()) {
        return;
    }
    btnPlay.isOpen();
    auto optProgress = sliderProgress.slide(playAudioProgress);
    if (optProgress.has_value()) {
        playAudioProgress = *optProgress;
    }
}

void TabCard::handleCardSubmission(widget::Button& btnReveal, widget::Button& btnSubmit, widget::Button& btnNext)
{
    if (displayText == nullptr) {
        return;
    }

    if (displayVocables == nullptr) {
        if (btnNext.clicked()) {
            signalProceed->set(Proceed::next);
            if (!cardAudioInfo.nextId.has_value()) {
                mode = Mode::shuffle;
            }
        }
    } else if (revealVocables) {
        if (btnSubmit.clicked()) {
            revealVocables = false;
            switch (mode) {
            case Mode::shuffle:
                signalProceed->set(Proceed::submit_walkTree);
                break;
            case Mode::story:
                signalProceed->set(Proceed::submit_next);
                if (!cardAudioInfo.nextId.has_value()) {
                    mode = Mode::shuffle;
                }
                break;
            }
        }
    } else {
        if (btnReveal.clicked()) {
            revealVocables = true;
        }
    }
}

void TabCard::handleMode(widget::ToggleButtonGroup& tbgMode)
{
    if (!cardAudioInfo.previousId.has_value() && !cardAudioInfo.nextId.has_value()) {
        mode = Mode::shuffle;
        return;
    }
    mode = static_cast<Mode>(tbgMode.Active(static_cast<std::size_t>(mode)));
}

void TabCard::handleNextPrevious(widget::ImageButton& btnFirst,
                                 widget::ImageButton& btnPrevious,
                                 widget::ImageButton& btnFollowing,
                                 widget::ImageButton& btnLast)
{
    if (!cardAudioInfo.firstId.has_value()
        && !cardAudioInfo.previousId.has_value()
        && !cardAudioInfo.nextId.has_value()
        && !cardAudioInfo.lastId.has_value()) {
        return;
    }
    btnFirst.setSensitive(cardAudioInfo.firstId.has_value());
    btnPrevious.setSensitive(cardAudioInfo.previousId.has_value());
    btnFollowing.setSensitive(cardAudioInfo.nextId.has_value());
    btnLast.setSensitive(cardAudioInfo.lastId.has_value());
    if (btnFirst.clicked()) {
        mode = Mode::story;
        signalProceed->set(Proceed::first);
    }
    if (btnPrevious.clicked()) {
        mode = Mode::story;
        signalProceed->set(Proceed::previous);
    }
    if (btnFollowing.clicked()) {
        mode = Mode::story;
        signalProceed->set(Proceed::next);
    }
    if (btnLast.clicked()) {
        mode = Mode::story;
        signalProceed->set(Proceed::last);
    }
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
