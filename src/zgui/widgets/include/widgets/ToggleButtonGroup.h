#pragma once
#include "Box.h"
#include "Widget.h"

#include <context/Texture.h>

#include <cstddef>
#include <initializer_list>
#include <magic_enum.hpp>
#include <memory>
#include <string>

namespace widget {
class ToggleButtonGroup : public Widget
{
    friend class Box;
    void setup(std::initializer_list<context::Image> images);
    void setup(std::initializer_list<std::string> labels);

public:
    ToggleButtonGroup(WidgetInit init);

    ~ToggleButtonGroup() override = default;
    ToggleButtonGroup(const ToggleButtonGroup&) = default;
    ToggleButtonGroup(ToggleButtonGroup&&) = default;
    auto operator=(const ToggleButtonGroup&) -> ToggleButtonGroup& = default;
    auto operator=(ToggleButtonGroup&&) -> ToggleButtonGroup& = default;

    auto getActive() -> std::size_t;
    auto arrange() -> bool override;

protected:
    auto calculateSize() const -> WidgetSize override;
private:
    auto createBox() -> std::shared_ptr<Box>;
    std::size_t active{0};
    std::shared_ptr<Box> box;
};
} // namespace widget
