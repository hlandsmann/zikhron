#pragma once
#include <spaced_repetition/AsyncTreeWalker.h>

#include <folly/executors/ManualExecutor.h>
#include <folly/experimental/coro/Task.h>

#include <memory>

class CardDisplay
{
public:
    CardDisplay(std::shared_ptr<folly::ManualExecutor> synchronousExecutor,
                std::shared_ptr<sr::AsyncTreeWalker> asyncTreeWalker);

private:
    auto feedingTask(std::shared_ptr<sr::AsyncTreeWalker> asyncTreeWalker) -> folly::coro::Task<>;

    std::shared_ptr<folly::ManualExecutor> synchronousExecutor;
};
