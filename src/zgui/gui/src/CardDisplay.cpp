#include <spaced_repetition/AsyncTreeWalker.h>
#include <CardDisplay.h>
#include <folly/executors/ManualExecutor.h>
#include <folly/experimental/coro/Task.h>

#include <memory>
#include <utility>

CardDisplay::CardDisplay(std::shared_ptr<folly::ManualExecutor> _synchronousExecutor,
                         std::shared_ptr<sr::AsyncTreeWalker> _asyncTreeWalker)
    : synchronousExecutor{std::move(_synchronousExecutor)}

{
    feedingTask(std::move(_asyncTreeWalker)).semi().via(synchronousExecutor.get());
}

auto CardDisplay::feedingTask(std::shared_ptr<sr::AsyncTreeWalker> asyncTreeWalker) -> folly::coro::Task<>
{
    auto cardMeta = co_await asyncTreeWalker->getNextCardChoice();

    co_return;
}