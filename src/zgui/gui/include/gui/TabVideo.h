#pragma once
#include "GroupAdd.h"

#include <context/WidgetId.h>
#include <widgets/Layer.h>

#include <memory>

namespace gui {
class TabVideo
{
public:
    TabVideo() = default;
    TabVideo(const TabVideo&) = delete;
    TabVideo(TabVideo&&) = delete;
    auto operator=(const TabVideo&) -> TabVideo& = delete;
    auto operator=(TabVideo&&) -> TabVideo& = delete;
    virtual ~TabVideo() = default;

    void setUp(std::shared_ptr<widget::Layer> layer);
    void displayOnLayer(widget::Layer& layer);

private:
    constexpr static widget::BoxCfg gridCfg = {.padding = {},
                                               .paddingHorizontal = {},
                                               .paddingVertical = {},
                                               .border = 16.F};
    std::unique_ptr<GroupAdd> groupAdd;
    context::WidgetId windowId{};
};

} // namespace gui
