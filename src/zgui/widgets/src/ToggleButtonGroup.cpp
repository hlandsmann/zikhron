#include <Button.h>
#include <memory>
#include <ImageButton.h>
#include <ToggleButtonGroup.h>
#include <Widget.h>
#include <context/Texture.h>
#include <context/imglog.h>
#include <utils/variant_cast.h>

#include <cstddef>
#include <initializer_list>
#include <string>
#include <variant>

namespace widget {
void ToggleButtonGroup::setup(std::initializer_list<context::Image> images)
{
    box = std::make_shared<Box>(getThemePtr(), PassiveOrientation(), std::weak_ptr{shared_from_this()});
    box->setPadding(0.F);
    for (const auto& image : images) {
        box->add<ImageButton>(layout::Align::start, image);
    }
}

void ToggleButtonGroup::setup(std::initializer_list<std::string> labels)
{
    box = std::make_shared<Box>(getThemePtr(), PassiveOrientation(), std::weak_ptr{shared_from_this()});
    box->setPadding(0.F);
    for (const auto& label : labels) {
        box->add<Button>(layout::Align::start, label);
    }
}

ToggleButtonGroup::ToggleButtonGroup(WidgetInit init)
    : Widget{init}
{
}


auto ToggleButtonGroup::calculateSize() const -> WidgetSize
{
    return box->getWidgetSize();
}

auto ToggleButtonGroup::getActive() -> std::size_t
{
    box->start();
    for (std::size_t index = 0; index < box->numberOfWidgets(); index++) {
        auto& widget = box->next<Widget>();
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

auto ToggleButtonGroup::arrange() -> bool
{
    return box->arrange();
}

} // namespace widget
