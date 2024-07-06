#include "GroupAdd.h"

#include <widgets/Box.h>
#include <widgets/Child.h>
#include <widgets/Grid.h>
#include <widgets/ImageButton.h>

#include <memory>
#include <utility>

namespace gui {

GroupAdd::GroupAdd(std::shared_ptr<widget::Grid> _grid)
    : grid{std::move(_grid)}
{
    auto& child = *grid->add<widget::Child>(Align::start, s_width, "group_add");
    childWidgetId = child.getWidgetId();
    auto& box = *child.add<widget::Box>(Align::start, boxCfg, widget::Orientation::horizontal);
    box.add<widget::ImageButton>(Align::start, context::Image::list_add);
}

auto GroupAdd::draw() -> bool
{
    auto& child = grid->getWidget<widget::Child>(childWidgetId);
    auto drop = child.dropChild();

    child.start();
    auto& box = child.next<widget::Box>();
    box.start();
    auto& button = box.next<widget::ImageButton>();
    return button.clicked();
}

} // namespace gui
