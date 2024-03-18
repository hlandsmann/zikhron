#include "TabCard.h"

#include <DisplayText.h>
#include <DisplayVocables.h>
#include <context/imglog.h>
#include <spaced_repetition/AsyncTreeWalker.h>
#include <spdlog/spdlog.h>
#include <widgets/Box.h>
#include <widgets/Button.h>
#include <widgets/Layer.h>
#include <widgets/TextTokenSeq.h>
#include <widgets/Window.h>
#include <widgets/detail/Widget.h>

#include <kocoro/kocoro.hpp>
#include <memory>
#include <ranges>
#include <utility>

namespace ranges = std::ranges;
namespace views = std::views;

namespace gui {

TabCard::TabCard(std::shared_ptr<kocoro::SynchronousExecutor> _synchronousExecutor,
                 std::shared_ptr<sr::AsyncTreeWalker> _asyncTreeWalker)
    : executor{std::move(_synchronousExecutor)}
    , signalVocIdEase{executor->makeVolatileSignal<VocableId_Ease>()}
    , signalCardBox{executor->makePersistentSignal<BoxPtr>()}
// , cardBoxContract{folly::makePromiseContract<std::shared_ptr<widget::Box>>()}

{
    // feedingTask(std::move(_asyncTreeWalker)).semi().via(synchronousExecutor.get());
    // signalVocIdEase->set(3);
    executor->startCoro(feedingTask(std::move(_asyncTreeWalker)));
}

void TabCard::setUp(std::shared_ptr<widget::Layer> layer)
{
    using namespace widget::layout;
    auto box = layer->add<widget::Box>(Align::start, widget::Orientation::vertical);
    boxId = box->getWidgetId();

    box->setName("DisplayCard_box");
    // box->setExpandType(width_expand, height_expand);
    auto& cardWindow = *box->add<widget::Window>(Align::start, width_expand, height_expand, "card_text");
    auto cardBox = cardWindow.add<widget::Box>(Align::start, widget::Orientation::vertical);
    cardBox->setName("cardBox");
    // cardBox->setFlipChildrensOrientation(false);
    signalCardBox->set(cardBox);

    // auto& testWindow = *box->add<widget::Window>(Align::center, ExpandType::width_expand, ExpandType::height_expand, "test_window");
    // auto& testBox = *testWindow.add<widget::Box>(Align::start, widget::Orientation::horizontal);

    auto& ctrlWindow = *box->add<widget::Window>(Align::end, ExpandType::width_expand, ExpandType::height_fixed, "card_ctrl");
    auto& ctrlBox = *ctrlWindow.add<widget::Box>(Align::start, widget::Orientation::horizontal);
    ctrlBox.setName("ctrlBox");
    ctrlBox.add<widget::Button>(Align::center, "hello");

    spdlog::info("setUp");
}

void TabCard::displayOnLayer(widget::Layer& layer)
{
    auto box = layer.getWidget<widget::Box>(boxId);
    box.start();
    doCardWindow(box.next<widget::Window>());
    // doTestWindow(box.next<widget::Window>());
    doCtrlWindow(box.next<widget::Window>());

    // auto& box = window.getBox();
    // // auto size = box.getExpandedSize();
    // // auto size = box.getWidgetSize();
    // // spdlog::critical("w: {}, h: {}, we: {}, he: {}", size.width, size.height, size.widthType, size.heightType);
    // box.start();
    // auto& cardBox = box.next<widget::Box>();
    // cardBox.start();
    // if (!cardBox.isLast()) {
    //     cardBox.next<widget::TextTokenSeq>().draw();
    // }
    // // imglog::log("width: {}, height: {}", window.getWidgetSize().width, window.getWidgetSize().height);
    // // while (!cardBox.isLast()) {
    // // cardBox.next<widget::Button>().clicked();
    // // }
}

auto TabCard::feedingTask(std::shared_ptr<sr::AsyncTreeWalker> asyncTreeWalker) -> kocoro::Task<>
{
    using namespace widget::layout;
    auto cardBox = co_await *signalCardBox;
    auto cardLayer = cardBox->add<widget::Layer>(Align::start);
    auto vocableLayer = cardBox->add<widget::Layer>(Align::start);
    cardLayer->setName("cardLayer");
    vocableLayer->setName("vocableLayer");
    cardLayer->setExpandType(width_expand, height_fixed);

    while (true) {
        displayText.reset();
        displayVocables.reset();
        cardLayer->clear();
        vocableLayer->clear();

        auto cardMeta = co_await asyncTreeWalker->getNextCardChoice();
        const auto& zh_dictionary = cardMeta.getDictionary();
        auto vocId_ease = cardMeta.getRelevantEase();
        auto tokenText = cardMeta.getStudyTokenText();

        auto orderedVocId_ease = tokenText->setupActiveVocableIds(vocId_ease);
        displayText = std::make_unique<DisplayText>(cardLayer, std::move(tokenText));
        displayVocables = std::make_unique<DisplayVocables>(vocableLayer, zh_dictionary, std::move(orderedVocId_ease));

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

void TabCard::doTestWindow(widget::Window& cardWindow)
{
    auto droppedWindow = cardWindow.dropWindow();
    // auto& cardBox = cardWindow.next<widget::Box>();
    // cardBox.start();
}

void TabCard::doCardWindow(widget::Window& cardWindow)
{
    auto droppedWindow = cardWindow.dropWindow();
    if (displayText) {
        displayText->draw();
    }
    if (displayVocables) {
        displayVocables->draw();
    }
    // auto& cardLayer = cardWindow.next<widget::Layer>();
    // cardLayer.start();
    // if (!cardLayer.isLast()) {
    //     cardLayer.next<widget::TextTokenSeq>().draw();
    // }
}

void TabCard::doCtrlWindow(widget::Window& ctrlWindow)
{
    auto droppedWindow = ctrlWindow.dropWindow();
    ctrlWindow.start();
    auto& box = ctrlWindow.next<widget::Box>();
    box.start();
    box.next<widget::Button>().clicked();
}

} // namespace gui
