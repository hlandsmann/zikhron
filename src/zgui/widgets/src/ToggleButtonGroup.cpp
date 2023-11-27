#include <Button.h>
#include <ImageButton.h>
#include <ToggleButtonGroup.h>
#include <Widget.h>
#include <context/Texture.h>
#include <utils/variant_cast.h>

#include <cstddef>
#include <initializer_list>
#include <string>
#include <variant>

namespace widget {
ToggleButtonGroup::ToggleButtonGroup(WidgetInit init,
                                     std::initializer_list<context::Image> images)
    : Widget<ToggleButtonGroup>{init}
    , box{init}
{
    box.setPadding(0.F);
    for (const auto& image : images) {
        box.add<ImageButton>(layout::Align::start, image);
    }
}

ToggleButtonGroup::ToggleButtonGroup(const WidgetInit& init,
                                     std::initializer_list<std::string> labels)
    : Widget<ToggleButtonGroup>{init}
    , box{init}
{
    for (const auto& label : labels) {
        box.add<Button>(layout::Align::start, label);
    }
}

auto ToggleButtonGroup::calculateSize() const -> WidgetSize
{
    return box.getWidgetSize();
}

auto ToggleButtonGroup::getActive() -> std::size_t
{
    box.arrange();
    for (std::size_t index = 0; index < box.numberOfWidgets(); index++) {
        auto& widget = box.next<WidgetBase>();
        auto buttonVariant = utl::variant_cast<Button, ImageButton>(&widget);
        // auto& button = dynamic_cast<ImageButton&>(widget);
        if (std::visit([&](auto* button) {
                button->setChecked(active == index);
                return button->clicked();
            },
                       buttonVariant)) {
            active = index;
        }
    }
    return active;
}

} // namespace widget
