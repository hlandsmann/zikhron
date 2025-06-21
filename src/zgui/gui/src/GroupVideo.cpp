#include "GroupVideo.h"

#include <context/Fonts.h>
#include <database/VideoSet.h>
#include <theme/Sizes.h>
#include <widgets/Box.h>
#include <widgets/Child.h>
#include <widgets/Grid.h>
#include <widgets/Image.h>
#include <widgets/ImageButton.h>
#include <widgets/Label.h>

#include <memory>
#include <utility>

namespace gui {

GroupVideo::GroupVideo(std::shared_ptr<widget::Grid> _grid, database::VideoSetPtr _videoSet)
    : grid{std::move(_grid)}
    , videoSet{std::move(_videoSet)}
{
    using namespace widget::layout;
    auto& child = *grid->add<widget::Child>(Align::start, 0, "group_video");
    childWidgetId = child.getWidgetId();
    auto& box = *child.add<widget::Box>(Align::start, boxCfg, widget::Orientation::vertical);
    box.add<widget::Label>(Align::start, videoSet->getName(), context::FontType::chineseSmall);
    const auto& cover = videoSet->getCover();
    if (!cover.empty()) {
        box.add<widget::Image>(Align::start, cover);
    }
    auto& boxVideoChoice = *box.add<widget::Box>(Align::start, boxCfgChoice, widget::Orientation::horizontal);
    boxVideoChoice.setExpandType(width_expand, height_fixed);
    boxVideoChoice.add<widget::Label>(Align::start, "", context::FontType::chineseSmall);
    boxVideoChoice.add<widget::ImageButton>(Align::end, context::Image::arrow_left);
    boxVideoChoice.add<widget::ImageButton>(Align::end, context::Image::arrow_right);
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

    if (!videoSet->getCover().empty()) {
        box.next<widget::Image>().draw();
    }

    auto& boxVideoChoice = box.next<widget::Box>();
    boxVideoChoice.start();
    auto& videoChoiceLabel = boxVideoChoice.next<widget::Label>();

    auto& buttonPrevious = boxVideoChoice.next<widget::ImageButton>();
    auto& buttonNext = boxVideoChoice.next<widget::ImageButton>();

    const auto& [choice, video] = videoSet->getChoice();
    videoChoiceLabel.setText(video->getName());
    videoChoiceLabel.draw();
    buttonPrevious.setSensitive(choice > 0);
    if (buttonPrevious.clicked()) {
        videoSet->setChoice(choice - 1);
    }
    buttonNext.setSensitive(choice < videoSet->getVideos().size() - 1);
    if (buttonNext.clicked()) {
        videoSet->setChoice(choice + 1);
    }

    auto& button = box.next<widget::ImageButton>();
    // spdlog::info("widgetId: {}", button.getWidgetId());
    return button.clicked();
}

auto GroupVideo::getVideoSet() const -> database::VideoSetPtr
{
    return videoSet;
}

} // namespace gui
