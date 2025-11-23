#pragma once
#include "Box.h"
#include "detail/Widget.h" // IWYU pragma: export detail/Widget.h

#include <context/Texture.h>

#include <cstddef>
#include <initializer_list>
#include <magic_enum/magic_enum.hpp>
#include <memory>
#include <string>

namespace widget {
class ToggleButtonGroup : public Widget
{
    template<class T>
    friend class MetaBox;
    friend class Widget;
    void setup(Orientation orientation, std::initializer_list<context::Image> images);
    void setup(Orientation orientation, std::initializer_list<std::string> labels);

public:
    ToggleButtonGroup(WidgetInit init);
    ~ToggleButtonGroup() override = default;

    ToggleButtonGroup(const ToggleButtonGroup&) = delete;
    ToggleButtonGroup(ToggleButtonGroup&&) = delete;
    auto operator=(const ToggleButtonGroup&) -> ToggleButtonGroup& = delete;
    auto operator=(ToggleButtonGroup&&) -> ToggleButtonGroup& = delete;

    auto Active(unsigned active) -> unsigned;

    template<class EnumType>
    auto Active(EnumType _active) -> EnumType
    {
        return static_cast<EnumType>(Active(static_cast<unsigned>(_active)));
    }

    auto getActive() -> unsigned;
    auto arrange(const layout::Rect& /* rect */) -> bool override;
    void setExpandType(layout::ExpandType expandWidth, layout::ExpandType expandHeight);

private:
    auto calculateSize() const -> WidgetSize override;
    auto getWidgetSizeFromRect(const layout::Rect& rect) -> WidgetSize override;

    unsigned active{0};
    std::shared_ptr<Box> box;
    Orientation orientation{};
};
} // namespace widget
