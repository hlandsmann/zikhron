#pragma once
#include "DisplayText.h"
#include <annotation/Ease.h>
#include <context/WidgetIdGenerator.h>
#include <misc/Identifier.h>
#include <spaced_repetition/AsyncTreeWalker.h>
#include <widgets/Box.h>
#include <widgets/Layer.h>
#include <widgets/Window.h>
#include <widgets/detail/Widget.h>

#include <kocoro/kocoro.hpp>
#include <map>
#include <memory>

namespace gui {

class TabCard
{
    using Align = widget::layout::Align;
    using ExpandType = widget::layout::ExpandType;

public:
    TabCard(std::shared_ptr<kocoro::SynchronousExecutor> _synchronousExecutor,
            std::shared_ptr<sr::AsyncTreeWalker> asyncTreeWalker);
    void setUp(std::shared_ptr<widget::Layer> layer);
    void displayOnLayer(widget::Layer& layer);

private:
    using VocableId_Ease = std::map<VocableId, Ease>;
    auto feedingTask(std::shared_ptr<sr::AsyncTreeWalker> asyncTreeWalker) -> kocoro::Task<>;
    static void doTestWindow(widget::Window& cardWindow);
    void doCardWindow(widget::Window& cardWindow);
    void doCtrlWindow(widget::Window& ctrlWindow);

    std::shared_ptr<kocoro::SynchronousExecutor> executor;

    std::shared_ptr<DisplayText> displayText;

    using LayerPtr = std::shared_ptr<widget::Layer>;
    std::shared_ptr<kocoro::VolatileSignal<VocableId_Ease>> signalVocIdEase;
    std::shared_ptr<kocoro::PersistentSignal<LayerPtr>> signalCardLayer;
    context::WidgetId boxId{};
};
} // namespace gui