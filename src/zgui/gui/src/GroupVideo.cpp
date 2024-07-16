#include "GroupVideo.h"

#include <context/Fonts.h>
#include <database/VideoPack.h>
#include <theme/Sizes.h>
#include <widgets/Box.h>
#include <widgets/Child.h>
#include <widgets/Grid.h>
#include <widgets/ImageButton.h>
#include <widgets/Label.h>

#include <memory>
#include <utility>

namespace gui {

GroupVideo::GroupVideo(std::shared_ptr<widget::Grid> _grid, database::VideoPackPtr _videoPack)
    : grid{std::move(_grid)}
    , videoPack{std::move(_videoPack)}
{
    auto& child = *grid->add<widget::Child>(Align::start, Sizes::group, "group_video");
    childWidgetId = child.getWidgetId();
    auto& box = *child.add<widget::Box>(Align::start, boxCfg, widget::Orientation::vertical);
    box.add<widget::Label>(Align::start, videoPack->getName(), context::FontType::chineseSmall);
    box.add<widget::ImageButton>(Align::start, context::Image::media_playback_start);
}

auto GroupVideo::draw() -> bool
{
    auto& child = grid->getWidget<widget::Child>(childWidgetId);
    auto drop = child.dropChild();

    child.start();
    auto& box = child.next<widget::Box>();
    box.start();
    box.next<widget::Label>().draw();
    auto& button = box.next<widget::ImageButton>();
    // spdlog::info("widgetId: {}", button.getWidgetId());
    return button.clicked();
}

} // namespace gui
