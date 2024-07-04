#include "TabVideo.h"

#include <widgets/Layer.h>
#include <widgets/Window.h>

#include <memory>

namespace gui {
void TabVideo::setUp(std::shared_ptr<widget::Layer> layer)
{
    using namespace widget::layout;
    auto& cardWindow = *layer->add<widget::Window>(Align::start, width_expand, height_expand, "card_text");
    windowId = cardWindow.getWidgetId();

}

void TabVideo::displayOnLayer(widget::Layer& layer)
{
    auto window = layer.getWidget<widget::Window>(windowId);
    auto droppedWindow = window.dropWindow();
}
} // namespace gui
