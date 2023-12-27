#include <CardDisplay.h>
#include <context/imglog.h>
#include <spaced_repetition/AsyncTreeWalker.h>
#include <spdlog/spdlog.h>
#include <widgets/Box.h>
#include <widgets/Button.h>
#include <widgets/TextTokenSeq.h>
#include <widgets/Widget.h>
#include <widgets/Window.h>

#include <kocoro/kocoro.hpp>
#include <memory>
#include <utility>

CardDisplay::CardDisplay(std::shared_ptr<kocoro::SynchronousExecutor> _synchronousExecutor,
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

void CardDisplay::arrange(widget::Window& window)
{
    using Align = widget::layout::Align;
    auto& box = window.getLayout();
    auto cardBox = box.add<widget::Box>(Align::start);
    signalCardBox->set(cardBox);
    spdlog::info("arrange");
}

void CardDisplay::displayOnWindow(widget::Window& window)
{
    auto droppedWindow = window.dropWindow();

    auto& box = window.getLayout();
    box.arrange();
    auto& cardBox = box.next<widget::Box>();
    cardBox.arrange();
    imglog::log("width: {}, height: {}", window.getWidgetSize().width, window.getWidgetSize().height);
    // while (!cardBox.isLast()) {
    // cardBox.next<widget::Button>().clicked();
    // }
}

auto CardDisplay::setUpBoxTask(std::shared_ptr<widget::Box> cardBox) -> kocoro::Task<>
{
    // spdlog::critical("setUpBoxTask");
    // cardBoxPromise.setValue(std::move(cardBox));
    // spdlog::critical("exit");
    co_return;
}

auto CardDisplay::feedingTask(std::shared_ptr<sr::AsyncTreeWalker> asyncTreeWalker) -> kocoro::Task<>
{
    using Align = widget::layout::Align;
    auto& cardBox = *co_await *signalCardBox;

    while (true) {
        auto cardMeta = co_await asyncTreeWalker->getNextCardChoice();
        auto tokenText = cardMeta.getStudyTokenText();
        cardBox.clear();
        cardBox.add<widget::TextTokenSeq>(Align::start, tokenText.getParagraph());

        // for (const auto& token : tokenText.getParagraph()) {
        //     spdlog::info("{}", token.getValue());
        // }
        // cardBox->clear();
        // cardBox->add<widget::TextTokenSeq>(Align::start, tokenText.getParagraph());

        // signalVocIdEase
        auto& signalVoIdEase = *signalVocIdEase;
        const auto& vocIdEase = co_await signalVoIdEase;
    }

    co_return;
}
