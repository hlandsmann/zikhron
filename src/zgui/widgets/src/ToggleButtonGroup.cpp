#include <Button.h>
#include <ImageButton.h>
#include <ToggleButtonGroup.h>
#include <context/Texture.h>
#include <context/imglog.h>
#include <detail/Widget.h>
#include <utils/variant_cast.h>

#include <cstddef>
#include <initializer_list>
#include <memory>
#include <string>
#include <variant>

namespace widget {
void ToggleButtonGroup::setup(std::initializer_list<context::Image> images)
{
    box = create<Box>();
    box->setPadding(0.F);
    for (const auto& image : images) {
        box->add<ImageButton>(layout::Align::start, image);
    }
}

void ToggleButtonGroup::setup(std::initializer_list<std::string> labels)
{
    box = create<Box>();
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

auto ToggleButtonGroup::arrange(const layout::Rect& rect) -> bool
{
    return box->arrange(rect);
}

} // namespace widget
