#pragma once
#include "Box.h"
#include "detail/Widget.h" // IWYU pragma: export detail/Widget.h

#include <context/Texture.h>

#include <cstddef>
#include <initializer_list>
#include <magic_enum.hpp>
#include <memory>
#include <string>

namespace widget {
class ToggleButtonGroup : public Widget
{
public:
    void setup(Orientation orientation, std::initializer_list<context::Image> images);
    void setup(Orientation orientation, std::initializer_list<std::string> labels);
    ToggleButtonGroup(WidgetInit init);

    ~ToggleButtonGroup() override = default;
    ToggleButtonGroup(const ToggleButtonGroup&) = default;
    ToggleButtonGroup(ToggleButtonGroup&&) = default;
    auto operator=(const ToggleButtonGroup&) -> ToggleButtonGroup& = default;
    auto operator=(ToggleButtonGroup&&) -> ToggleButtonGroup& = default;

    auto Active(std::size_t active) -> std::size_t;
    auto getActive() -> std::size_t;
    auto arrange(const layout::Rect& /* rect */) -> bool override;

protected:
    auto calculateSize() const -> WidgetSize override;

private:
    std::size_t active{0};
    std::shared_ptr<Box> box;
};
} // namespace widget
