#include <VocableOverlay.h>
#include <spdlog/spdlog.h>
#include <algorithm>
#include <boost/range/combine.hpp>
#include <ranges>

namespace ranges = std::ranges;

VocableOverlay::VocableOverlay(const VocableOverlayInit& init)
    : entries(std::move(init.entries)), choice_entry(init.choice_entry) {
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

    if (entries.size() > 1) {
        expandEntriesBtn.set_label(">");
        expandEntriesBtn.signal_clicked().connect(
            [this, x = init.x, y = init.y, x_max = init.x_max, y_max = init.y_max]() {
                if (expandEntriesBtn.property_active()) {
                    expandEntriesBtn.set_label("v");
                    entryChoiceGrid.set_visible(true);
                    show(x, y, x_max, y_max);
                } else {
                    expandEntriesBtn.set_label(">");
                    entryChoiceGrid.set_visible(false);
                    show(x, y, x_max, y_max);
                }
            });
        expandEntriesBtn.set_valign(Gtk::Align::END);
        expandEntriesBtn.set_halign(Gtk::Align::START);
        currentGrid.attach(expandEntriesBtn, 0, 1);
    }

    if (entries[choice_entry].meanings.size() > 1) {
        expandMeaningsBtn.set_label("...");
        expandMeaningsBtn.signal_clicked().connect(
            [this, x = init.x, y = init.y, x_max = init.x_max, y_max = init.y_max]() {
                if (expandMeaningsBtn.property_active()) {
                    meaningChoiceBox.set_visible(true);
                    show(x, y, x_max, y_max);
                } else {
                    meaningChoiceBox.set_visible(false);
                    show(x, y, x_max, y_max);
                }
            });
        expandMeaningsBtn.set_valign(Gtk::Align::END);
        expandMeaningsBtn.set_halign(Gtk::Align::START);
        currentGrid.attach(expandMeaningsBtn, 3, 1);
    }

    currentGrid.set_row_spacing(10);
    currentGrid.set_column_spacing(20);
    currentGrid.set_margin(10);

    entryChoiceGrid.set_row_spacing(10);
    entryChoiceGrid.set_column_spacing(20);
    entryChoiceGrid.set_margin(10);

    meaningChoiceBox.set_spacing(10);
    meaningChoiceBox.set_margin(10);
    meaningChoiceBox.set_orientation(Gtk::Orientation::VERTICAL);

    setupTextDraw();

    show(init.x, init.y, init.x_max, init.y_max);
}

void VocableOverlay::setupTextDraw() {
    std::vector<std::string> meaning_multipleEntries;
    std::vector<std::string> pronounciations_vertical;
    ranges::transform(entries,
                      std::back_inserter(meaning_multipleEntries),
                      [](const ZH_Dictionary::Entry& entry) { return entry.meanings.front(); });
    ranges::transform(entries,
                      std::back_inserter(pronounciations_vertical),
                      [](const ZH_Dictionary::Entry& entry) { return entry.pronounciation; });
    auto list = {{entries.front().key},
                 {entries[choice_entry].pronounciation},
                 {entries[choice_entry].meanings.front()},
                 pronounciations_vertical,
                 meaning_multipleEntries,
                 entries[choice_entry].meanings};
    textDrawContainer.clear();
    ranges::transform(list | std::views::join,
                      std::back_inserter(textDrawContainer),
                      [this, index = 0](const std::string& str) mutable {
                          auto textDraw = std::make_unique<TextDraw>();
                          textDraw->setSpacing(fontSpacing);
                          textDraw->setFontSize(fontSize);
                          textDraw->setText(str);
                          textDraw->set_hexpand();
                          textDraw->set_hard_size_request();
                          index++;
                          return textDraw;
                      });
    auto textDrawIt = textDrawContainer.begin();
    currentGrid.attach(**(textDrawIt++), 0, 0, 3);
    currentGrid.attach(**(textDrawIt++), 1, 1);
    currentGrid.attach(**(textDrawIt++), 2, 1);
    box.append(currentGrid);

    textDrawPronounciations = std::span(textDrawIt, textDrawIt + entries.size());
    std::advance(textDrawIt, entries.size());
    textDrawMeaning_multipleEntries = std::span(textDrawIt, textDrawIt + entries.size());
    std::advance(textDrawIt, entries.size());
    textDrawMeanings_singleEntry = std::span(textDrawIt,
                                             textDrawIt + entries[choice_entry].meanings.size());
    int index = 0;
    for (const auto& [textDrawPronouncation, textDrawMeaning] :
         boost::combine(textDrawPronounciations, textDrawMeaning_multipleEntries)) {
        entryChoiceGrid.attach(*textDrawPronouncation, 0, index);
        entryChoiceGrid.attach(*textDrawMeaning, 1, index);
        index++;
    }
    setupTextdrawCallbacks();
    entryChoiceGrid.set_visible(false);
    box.append(entryChoiceGrid);
    for (const auto& textDrawMeaning : textDrawMeanings_singleEntry) {
        meaningChoiceBox.append(*textDrawMeaning);
    }
    meaningChoiceBox.set_visible(false);
    box.append(meaningChoiceBox);
}

void VocableOverlay::show(int x, int y, int x_max, int y_max) {
    if (box.is_ancestor(*this)) {
        remove(box);
    }

    if (textDrawContainer.empty())
        return;

    int minWidth, naturalWidth, minBaseline, naturalBaseline;
    box.measure(Gtk::Orientation::HORIZONTAL, -1, minWidth, naturalWidth, minBaseline, naturalBaseline);

    if (naturalWidth > maxWidth) {
        box.set_size_request(maxWidth, -1);
        naturalWidth = maxWidth;
    } else
        box.set_size_request(naturalWidth, -1);
    x = std::max(0, std::min(x, x_max - naturalWidth));

    int minHeight, naturalHeight;
    box.measure(Gtk::Orientation::VERTICAL, -1, minHeight, naturalHeight, minBaseline, naturalBaseline);

    y = std::max(0, std::min(y, y_max - naturalHeight));

    put(box, double(x), double(y));
    x_pos = x;
    y_pos = y;
}

void VocableOverlay::setupTextdrawCallbacks() {
    int index = 0;
    for (const auto& [textDrawMeaning, textDrawPronounciation] :
         boost::combine(textDrawPronounciations, textDrawMeaning_multipleEntries)) {
        auto mouseClick = [this, index](auto) { callback_click(index); };
        auto mouseLeave = [this, index] { callback_leave(index); };
        auto mouseMotion = [this, index](auto) { callback_motion(index); };
        textDrawPronounciation->signal_mouseClickByteIndex(mouseClick);
        textDrawMeaning->signal_mouseClickByteIndex(mouseClick);
        textDrawPronounciation->signal_mouseLeave(mouseLeave);
        textDrawMeaning->signal_mouseLeave(mouseLeave);
        textDrawPronounciation->signal_hoverByteIndex(mouseMotion);
        textDrawMeaning->signal_hoverByteIndex(mouseMotion);
        index++;
    }
}

void VocableOverlay::callback_click(int textDrawIndex) {
    if (func_vocableChoice) {
        func_vocableChoice(textDrawIndex);
    }
}

void VocableOverlay::callback_motion(int textDrawIndex) {
    std::apply(
        [this, textDrawIndex](auto... color) {
            textDrawPronounciations[textDrawIndex]->setFontColor(color...);
            textDrawMeaning_multipleEntries[textDrawIndex]->setFontColor(color...);
        },
        markedColorRGB);
}

void VocableOverlay::callback_leave(int textDrawIndex) {
    textDrawPronounciations[textDrawIndex]->setFontColorDefault();
    textDrawMeaning_multipleEntries[textDrawIndex]->setFontColorDefault();
}
