#include "TabCard.h"

#include <DisplayText.h>
#include <DisplayVocables.h>
#include <VocableOverlay.h>
#include <context/imglog.h>
#include <spaced_repetition/AsyncTreeWalker.h>
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

#include <initializer_list>
#include <kocoro/kocoro.hpp>
#include <memory>
#include <ranges>
#include <string>
#include <utility>

namespace ranges = std::ranges;
namespace views = std::views;

namespace gui {

TabCard::TabCard(std::shared_ptr<kocoro::SynchronousExecutor> _synchronousExecutor,
                 std::shared_ptr<sr::AsyncTreeWalker> _asyncTreeWalker)
    : executor{std::move(_synchronousExecutor)}
    , signalVocIdEase{executor->makeVolatileSignal<VocableId_Ease>()}
    , signalCardBox{executor->makePersistentSignal<BoxPtr>()}
{
    executor->startCoro(feedingTask(std::move(_asyncTreeWalker)));
}

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
    auto dataBase = co_await asyncTreeWalker->getDataBase();
    auto wordDB = dataBase->getWordDB();

    while (true) {
        displayText.reset();
        displayVocables.reset();
        cardLayer->clear();
        vocableLayer->clear();

        // card 601 has problems

        auto cardMeta = co_await asyncTreeWalker->getNextCardChoice();
        // const auto& zh_dictionary = cardMeta.getDictionary();
        auto vocId_ease = cardMeta.getRelevantEase();
        auto tokenText = cardMeta.getStudyTokenText();

        auto orderedVocId_ease = tokenText->setupActiveVocableIds(vocId_ease);
        displayText = std::make_unique<DisplayText>(cardLayer, std::move(tokenText));
        displayVocables = std::make_unique<DisplayVocables>(vocableLayer, wordDB, std::move(orderedVocId_ease));

        // cardLayer->add<widget::TextTokenSeq>(Align::start, tokenText->getParagraph());

        // for (const auto& token : tokenText.getParagraph()) {
        //     spdlog::info("{}", token.getValue());
        // }

        // signalVocIdEase
        auto& signalVoIdEase = *signalVocIdEase;
        const auto& vocIdEase = co_await signalVoIdEase;
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
    if (displayVocables) {
        displayVocables->draw();
    }
    if (vocableOverlay) {
        vocableOverlay->draw();
        if (vocableOverlay->wasConfigured()) {
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
                                           std::initializer_list<std::string>{"single", "group"})
            ->setExpandType(width_fixed, height_adapt);

    ctrlBox.add<widget::Separator>(Align::end, 16.F, 0.F);
    ctrlBox.add<widget::ImageButton>(Align::end, Image::media_skip_backward);
    // ctrlBox.add<widget::Separator>(Align::end, 4.F, 0.F);
    ctrlBox.add<widget::ImageButton>(Align::end, Image::media_seek_backward);
    // ctrlBox.add<widget::Separator>(Align::end, 4.F, 0.F);
    ctrlBox.add<widget::ImageButton>(Align::end, Image::media_skip_forward);
    // ctrlBox.add<widget::Separator>(Align::end, 4.F, 0.F);
    ctrlBox.add<widget::ImageButton>(Align::end, Image::media_seek_forward);
    ctrlBox.add<widget::Separator>(Align::end, 16.F, 0.F);
    ctrlBox.add<widget::ImageButton>(Align::end, Image::document_save);
}

void TabCard::doCtrlWindow(widget::Window& ctrlWindow)
{
    auto droppedWindow = ctrlWindow.dropWindow();
    ctrlWindow.start();
    auto& box = ctrlWindow.next<widget::Box>();
    box.start();
    box.next<widget::ImageButton>().isOpen();
    // box.next<widget::Separator>();

    static float x = 0;
    auto optx = box.next<widget::MediaSlider>().slide(x);
    if (optx.has_value()) {
        x = *optx;
    }

    box.next<widget::Separator>();
    auto& layer = box.next<widget::Layer>();
    layer.start();
    layer.next<widget::Button>().clicked();

    box.next<widget::Separator>();
    box.next<widget::ToggleButtonGroup>().Active(0);
    box.next<widget::Separator>();
    box.next<widget::ImageButton>().clicked();
    // box.next<widget::Separator>();
    box.next<widget::ImageButton>().clicked();
    // box.next<widget::Separator>();
    box.next<widget::ImageButton>().clicked();
    // box.next<widget::Separator>();
    box.next<widget::ImageButton>().clicked();
    box.next<widget::Separator>();
    box.next<widget::ImageButton>().clicked();
}

} // namespace gui
