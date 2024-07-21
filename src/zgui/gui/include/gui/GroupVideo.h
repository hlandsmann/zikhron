#pragma once
#include <context/WidgetId.h>
#include <database/VideoPack.h>
#include <widgets/Grid.h>

#include <memory>

namespace gui {

class GroupVideo
{
    using Align = widget::layout::Align;
    constexpr static float s_width = 500;

public:
    GroupVideo(std::shared_ptr<widget::Grid> grid, database::VideoPackPtr videoPack);
    auto draw() -> bool;

    [[nodiscard]] auto getVideoPack() const -> database::VideoPackPtr;

private:
    constexpr static widget::BoxCfg boxCfg = {.padding = 8.F,
                                              .paddingHorizontal = {},
                                              .paddingVertical = {},
                                              .border = 8.F};
    context::WidgetId childWidgetId;
    std::shared_ptr<widget::Grid> grid;
    database::VideoPackPtr videoPack;
};

} // namespace gui
