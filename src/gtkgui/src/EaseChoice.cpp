#include <EaseChoice.h>
#include <spdlog/spdlog.h>
#include <algorithm>
#include <ranges>
#include <span>
namespace ranges = std::ranges;

EaseChoice::EaseChoice() {
    set_orientation(Gtk::Orientation::HORIZONTAL);
    const auto& easeList = {"Again", "Hard", "Normal", "Easy"};
    ranges::transform(easeList, std::back_inserter(easeButtons), [this](const std::string& easeName) {
        auto btn = std::make_unique<Gtk::ToggleButton>();
        btn->set_label(easeName);
        btn->signal_toggled().connect([this]() {
            auto it = ranges::find_if(
                easeButtons, [](const auto& toggleButton) { return toggleButton->property_active(); });
            if (it != easeButtons.end())
                ease = static_cast<Ease>(std::distance(easeButtons.begin(), it));
        });
        btn->set_valign(Gtk::Align::CENTER);
        this->append(*btn);
        return btn;
    });
    for (auto& btn : std::span(next(easeButtons.begin()), easeButtons.end())) {
        btn->set_group(*easeButtons.front());
    }
}

void EaseChoice::setEase(Ease ease_in) {
    ease = ease_in;
    easeButtons[static_cast<size_t>(ease)]->property_active() = true;
}

auto EaseChoice::getEase() const -> Ease { return ease; }
