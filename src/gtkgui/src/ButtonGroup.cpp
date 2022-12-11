#include <ButtonGroup.h>
#include <spdlog/spdlog.h>
#include <algorithm>
#include <ranges>
#include <span>
namespace ranges = std::ranges;

ButtonGroup::ButtonGroup(std::initializer_list<std::string> labels) {
    set_orientation(Gtk::Orientation::HORIZONTAL);
    ranges::transform(labels, std::back_inserter(buttons), [this](const std::string& label) {
        auto btn = std::make_unique<Gtk::ToggleButton>();
        btn->set_label(label);
        btn->signal_toggled().connect([this]() {
            auto it = ranges::find_if(
                buttons, [](const auto& toggleButton) { return toggleButton->property_active(); });
            if (it != buttons.end())
                active = static_cast<uint>(std::distance(buttons.begin(), it));
        });
        btn->set_valign(Gtk::Align::CENTER);
        this->append(*btn);
        return btn;
    });
    for (auto& btn : std::span(next(buttons.begin()), buttons.end())) {
        btn->set_group(*buttons.front());
    }
}

void ButtonGroup::setActive(uint active_in) {
    active = active_in;
    if (active >= buttons.size()) {
        spdlog::error("ButtonGroup setActive out-of-range - active {}", active);
        active = 0;
    }
    buttons[active]->property_active() = true;
}

auto ButtonGroup::getActive() const -> uint { return active; }

void ButtonGroup::observe_active(const std::function<void(uint)> observer) {
    auto activeObserver = active.observe(observer);
    observers.push(activeObserver);
}
