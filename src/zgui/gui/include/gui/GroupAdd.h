#pragma once
#include <context/WidgetId.h>
#include <widgets/Grid.h>

#include <memory>

namespace gui {

class GroupAdd
{
    using Align = widget::layout::Align;
    constexpr static float s_width = 500;

public:
    GroupAdd(std::shared_ptr<widget::Grid> grid);
    auto draw() -> bool;

private:
    constexpr static widget::BoxCfg boxCfg = {.padding = {},
                                              .paddingHorizontal = {},
                                              .paddingVertical = {},
                                              .border = 8.F};
    context::WidgetId childWidgetId;
    std::shared_ptr<widget::Grid> grid;
};

} // namespace gui
