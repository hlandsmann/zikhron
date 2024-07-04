#pragma once
#include <context/WidgetIdGenerator.h>
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
    context::WidgetId windowId{};
};

} // namespace gui
