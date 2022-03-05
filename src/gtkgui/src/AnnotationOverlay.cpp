#include <AnnotationOverlay.h>
#include <spdlog/spdlog.h>
#include <algorithm>
#include <ranges>
namespace ranges = std::ranges;

AnnotationOverlay::AnnotationOverlay(AnnotationOverlayInit& init)
    : marked(init.marked), unmarked(init.unmarked), activeChoice(init.activeChoice) {
    box.set_orientation(Gtk::Orientation::VERTICAL);
    // box.set_can_target();
    // box.set_can_focus();
    set_expand();
    set_valign(Gtk::Align::START);
    set_halign(Gtk::Align::START);
    setupTextDraw();
    show(init.x, init.y, init.x_max, init.y_max);
    set_can_target();
    set_size_request(init.x_max, init.y_max);

    clickController = Gtk::GestureClick::create();
    clickController->signal_pressed().connect([this](int /*key*/, double /*x*/, double /*y*/) {
        if (func_annotationChoice) {
            if (active != -1)
                func_annotationChoice(active);
            else
                func_annotationChoice({});
        }
    });
    add_controller(clickController);
}

void AnnotationOverlay::setupTextDraw() {
    auto list = {{activeChoice}, unmarked};
    ranges::transform(
        list | std::views::join,
        std::back_inserter(textDrawContainer),
        [this, index = -1](const std::string& unmark) mutable {
            auto textDraw = std::make_unique<TextDraw>();
            textDraw->setSpacing(0);
            textDraw->setFontSize(fontSize);
            textDraw->setDrawBorder();
            textDraw->setText(unmark);
            textDraw->set_hexpand();
            textDraw->set_size_request(textDraw->getWidth(), -1);
            textDraw->signal_hoverByteIndex([this, index](auto) { callback_motion(index); });
            textDraw->signal_mouseLeave([this, index]() { callback_leave(index); });
            box.append(*textDraw);
            index++;
            return textDraw;
        });
    textDrawAlternativeView = std::span(std::next(textDrawContainer.begin()), textDrawContainer.end());
}

void AnnotationOverlay::show(int x, int y, int x_max, int /* y_max */) {
    if (textDrawContainer.empty())
        return;
    int width = textDrawContainer.front()->getWidth();
    x = std::max(0, std::min(x, x_max - width));
    put(box, double(x), double(y));
}

void AnnotationOverlay::callback_motion(int index) {
    if (index == -1) {
        if (active != index) {
            textDrawContainer.front()->update_markup(activeChoice);
            textDrawAlternativeView[active]->update_markup(unmarked[active]);
            active = index;
        }
        return;
    }

    if (active != -1 && active != index) {
        auto& textDrawOld = textDrawAlternativeView[active];
        textDrawOld->update_markup(unmarked[active]);
    }
    if (active != index) {
        auto& textDraw = textDrawAlternativeView[index];
        textDraw->update_markup(marked[index]);
        textDrawContainer.front()->update_markup(marked[index]);
        active = index;
    }
}

void AnnotationOverlay::callback_leave(int index) {
    if (index == -1)
        return;
    auto& textDraw = textDrawAlternativeView[index];
    textDraw->update_markup(unmarked[index]);
    textDrawContainer.front()->update_markup(activeChoice);
    active = -1;
}
