#pragma once
#include <annotation/Ease.h>
#include <misc/Identifier.h>
#include <spaced_repetition/AsyncTreeWalker.h>
#include <widgets/Box.h>
#include <widgets/Window.h>

#include <kocoro/kocoro.hpp>
#include <map>
#include <memory>

class CardDisplay
{
public:
    CardDisplay(std::shared_ptr<kocoro::SynchronousExecutor> _synchronousExecutor,
                std::shared_ptr<sr::AsyncTreeWalker> asyncTreeWalker);
    void arrange(widget::Window& window);
    void displayOnWindow(widget::Window& window);

private:
    using VocableId_Ease = std::map<VocableId, Ease>;
    auto feedingTask(std::shared_ptr<sr::AsyncTreeWalker> asyncTreeWalker) -> kocoro::Task<>;
    auto setUpBoxTask(std::shared_ptr<widget::Box> cardBox) -> kocoro::Task<>;

    std::shared_ptr<kocoro::SynchronousExecutor> synchronousExecutor;

    using BoxPtr = std::shared_ptr<widget::Box>;
    std::shared_ptr<kocoro::Signal<VocableId_Ease>> signalVocIdEase;
};
