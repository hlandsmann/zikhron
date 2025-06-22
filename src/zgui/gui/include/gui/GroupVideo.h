#pragma once
#include <context/WidgetId.h>
#include <database/VideoSet.h>
#include <misc/Language.h>
#include <widgets/Grid.h>

#include <memory>

namespace gui {

class GroupVideo
{
    using Align = widget::layout::Align;

public:
    GroupVideo(std::shared_ptr<widget::Grid> grid, database::VideoSetPtr videoSet, Language language);
    auto draw() -> bool;

    [[nodiscard]] auto getVideoSet() const -> database::VideoSetPtr;

private:
    constexpr static widget::BoxCfg boxCfg = {.padding = 8.F,
                                              .paddingHorizontal = {},
                                              .paddingVertical = {},
                                              .border = 8.F};
    constexpr static widget::BoxCfg boxCfgChoice = {.padding = 4.F,
                                                    .paddingHorizontal = {},
                                                    .paddingVertical = {},
                                                    .border = 0.F};
    context::WidgetId childWidgetId;
    std::shared_ptr<widget::Grid> grid;
    database::VideoSetPtr videoSet;
};

} // namespace gui
