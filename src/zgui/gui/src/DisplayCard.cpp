#include <DisplayCard.h>
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
#include <utility>

DisplayCard::DisplayCard(std::shared_ptr<kocoro::SynchronousExecutor> _synchronousExecutor,
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

void DisplayCard::setUp(std::shared_ptr<widget::Layer> layer)
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

    auto& ctrlWindow = *box->add<widget::Window>(Align::end, ExpandType::width_expand, ExpandType::height_fixed, "card_ctrl");
    auto& ctrlBox = *ctrlWindow.add<widget::Box>(Align::start, widget::Orientation::horizontal);
    ctrlBox.setName("ctrlBox");
    ctrlBox.add<widget::Button>(Align::center, "hello");

    spdlog::info("setUp");
}

void DisplayCard::displayOnLayer(widget::Layer& layer)
{
    auto box = layer.getWidget<widget::Box>(boxId);
    box.start();
    doCardWindow(box.next<widget::Window>());
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

auto DisplayCard::feedingTask(std::shared_ptr<sr::AsyncTreeWalker> asyncTreeWalker) -> kocoro::Task<>
{
    auto& cardBox = *co_await *signalCardBox;

    while (true) {
        auto cardMeta = co_await asyncTreeWalker->getNextCardChoice();
        auto tokenText = cardMeta.getStudyTokenText();
        cardBox.clear();
        cardBox.add<widget::TextTokenSeq>(Align::start, tokenText.getParagraph());

        // for (const auto& token : tokenText.getParagraph()) {
        //     spdlog::info("{}", token.getValue());
        // }

        // signalVocIdEase
        auto& signalVoIdEase = *signalVocIdEase;
        const auto& vocIdEase = co_await signalVoIdEase;
    }

    co_return;
}

void DisplayCard::doCardWindow(widget::Window& cardWindow)
{
    auto droppedWindow = cardWindow.dropWindow();
    auto& cardBox = cardWindow.next<widget::Box>();
    cardBox.start();
    if (!cardBox.isLast()) {
        cardBox.next<widget::TextTokenSeq>().draw();
    }
}

void DisplayCard::doCtrlWindow(widget::Window& ctrlWindow)
{
    auto droppedWindow = ctrlWindow.dropWindow();
    ctrlWindow.start();
    auto& box = ctrlWindow.next<widget::Box>();
    box.start();
    box.next<widget::Button>().clicked();
}
