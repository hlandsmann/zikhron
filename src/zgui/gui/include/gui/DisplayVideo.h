#pragma once
#include <annotation/Ease.h>
#include <context/WidgetIdGenerator.h>
#include <misc/Identifier.h>
#include <spaced_repetition/AsyncTreeWalker.h>
#include <widgets/Box.h>
#include <widgets/Layer.h>
#include <widgets/Window.h>

#include <kocoro/kocoro.hpp>
#include <map>
#include <memory>

class DisplayVideo
{
public:
    DisplayVideo(std::shared_ptr<kocoro::SynchronousExecutor> _synchronousExecutor,
                 std::shared_ptr<sr::AsyncTreeWalker> asyncTreeWalker);
    void setUp(widget::Window& window);
    void displayOnWindow(widget::Window& window);

private:
    using VocableId_Ease = std::map<VocableId, Ease>;
    auto feedingTask(std::shared_ptr<sr::AsyncTreeWalker> asyncTreeWalker) -> kocoro::Task<>;

    std::shared_ptr<kocoro::SynchronousExecutor> executor;
};
