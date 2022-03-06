#include <VocableOverlay.h>
#include <spdlog/spdlog.h>
#include <algorithm>
#include <boost/range/combine.hpp>
#include <ranges>
namespace ranges = std::ranges;

VocableOverlay::VocableOverlay(const VocableOverlayInit& init)
    : key(init.key)
    , pronounciationChoice(init.pronounciationChoice)
    , meaningChoice(init.meaningChoice)
    , pronounciations(init.pronounciations)
    , meanings(init.meanings) {
    box.get_style_context()->add_class("overlay");
    box.set_orientation(Gtk::Orientation::VERTICAL);
    box.set_vexpand();

    set_expand();
    set_valign(Gtk::Align::START);
    set_halign(Gtk::Align::START);
    set_can_target();
    set_size_request(init.x_max, init.y_max);

    clickController = Gtk::GestureClick::create();
    clickController->signal_pressed().connect([this](int /*key*/, double x, double y) {
        auto width = box.get_size(Gtk::Orientation::HORIZONTAL);
        auto height = box.get_size(Gtk::Orientation::VERTICAL);

        if (x < x_pos || x > x_pos + width || y < y_pos || y > y_pos + height)
            if (func_vocableChoice) {
                func_vocableChoice({});
            }
    });
    add_controller(clickController);

    if (pronounciations.size() > 1) {
        expandBtn.set_label(">");
        expandBtn.signal_clicked().connect(
            [this, x = init.x, y = init.y, x_max = init.x_max, y_max = init.y_max]() {
                if (expandBtn.property_active()) {
                    expandBtn.set_label("v");
                    choiceGrid.set_visible(true);
                    show(x, y, x_max, y_max);
                } else {
                    expandBtn.set_label(">");
                    choiceGrid.set_visible(false);
                    show(x, y, x_max, y_max);
                }
            });
        expandBtn.set_valign(Gtk::Align::END);
        expandBtn.set_halign(Gtk::Align::START);
        currentGrid.attach(expandBtn, 0, 1);
    }

    currentGrid.set_row_spacing(10);
    currentGrid.set_column_spacing(20);
    currentGrid.set_margin(10);

    choiceGrid.set_row_spacing(10);
    choiceGrid.set_column_spacing(20);
    choiceGrid.set_margin(10);

    setupTextDraw();

    show(init.x, init.y, init.x_max, init.y_max);
}

void VocableOverlay::setupTextDraw() {
    auto list = {{key}, {pronounciationChoice}, {meaningChoice}, pronounciations, meanings};
    ranges::transform(
        list | std::views::join,
        std::back_inserter(textDrawContainer),
        [this, index = 0](const std::string& str) mutable {
            auto textDraw = std::make_unique<TextDraw>();
            textDraw->setSpacing(fontSpacing);
            textDraw->setFontSize(fontSize);
            // textDraw->setDrawBorder();
            textDraw->setText(str);
            textDraw->set_hexpand();
            textDraw->set_hard_size_request();
            textDraw->signal_hoverByteIndex([this, index](auto) { callback_motion(index); });
            textDraw->signal_mouseLeave([this, index]() { callback_leave(index); });
            index++;
            return textDraw;
        });
    auto textDrawIt = textDrawContainer.begin();
    currentGrid.attach(**(textDrawIt++), 0, 0, 3);
    currentGrid.attach(**(textDrawIt++), 1, 1);
    currentGrid.attach(**(textDrawIt++), 2, 1);
    box.append(currentGrid);

    textDrawPronounciations = std::span(textDrawIt, textDrawIt + pronounciations.size());
    textDrawMeanings = std::span(textDrawIt + pronounciations.size(), textDrawContainer.end());
    int index = 0;
    for (const auto& [textDrawPronouncation, textDrawMeaning] :
         boost::combine(textDrawPronounciations, textDrawMeanings)) {
        choiceGrid.attach(*textDrawPronouncation, 0, index);
        choiceGrid.attach(*textDrawMeaning, 1, index);
        index++;
    }
    choiceGrid.set_visible(false);
    box.append(choiceGrid);
}

void VocableOverlay::show(int x, int y, int x_max, int y_max) {
    if (textDrawContainer.empty())
        return;

    int minWidth, naturalWidth, minBaseline, naturalBaseline;
    box.measure(Gtk::Orientation::HORIZONTAL, -1, minWidth, naturalWidth, minBaseline, naturalBaseline);
    spdlog::warn("natural: {}", naturalWidth);

    if (naturalWidth > maxWidth) {
        box.set_size_request(maxWidth, -1);
        naturalWidth = maxWidth;

    } else
        box.set_size_request(naturalWidth, -1);
    x = std::max(0, std::min(x, x_max - naturalWidth));
    if (box.is_ancestor(*this)) {
        remove(box);
    }
    put(box, double(x), double(y));
    x_pos = x;
    y_pos = y;
}

void VocableOverlay::callback_motion(int index) {}

void VocableOverlay::callback_leave(int index) {}

// void VocableOverlay::VocableBin::size_allocate(int width, int height, int baseline) {
//     spdlog::warn("Size_allocate");
// }
// void VocableOverlay::VocableBin::measure_vfunc(Gtk::Orientation orientation,
//                                                int for_size,
//                                                int& minimum,
//                                                int& natural,
//                                                int& minimum_baseline,
//                                                int& natural_baseline) const {
//     spdlog::warn("measure");
// }
