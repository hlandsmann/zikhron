#pragma once

#include <TextDraw.h>
#include <dictionary/ZH_Dictionary.h>
#include <gtkmm.h>
#include <functional>
#include <optional>
#include <span>

struct VocableOverlayInit {
    std::vector<ZH_Dictionary::Entry> entries;
    size_t choice_entry;
    int x;
    int y;
    int x_max;
    int y_max;
};

class VocableOverlay : public Gtk::Fixed {
public:
    using TextDrawPtr = std::unique_ptr<TextDraw>;

    VocableOverlay(const VocableOverlayInit& init);
    void signal_vocableChoice(const std::function<void(std::optional<int> choice)>& functor) {
        func_vocableChoice = functor;
    };

private:
    constexpr static int fontSize = 20;
    constexpr static int fontSpacing = 10;
    constexpr static int maxWidth = 500;
    constexpr static std::tuple<double, double, double> markedColorRGB = {1.0, 1.0, 0.8};
    int x_pos = 0;
    int y_pos = 0;

    void setupTextDraw();
    void show(int x, int y, int x_max, int y_max);
    void setupTextdrawCallbacks();
    void callback_click(int textDrawIndex);
    void callback_leave(int textDrawIndex);
    void callback_motion(int textDrawIndex);

    Glib::RefPtr<Gtk::GestureClick> clickController;
    Gtk::Box box;
    Gtk::Grid currentGrid;
    Gtk::Grid entryChoiceGrid;
    Gtk::Box meaningChoiceBox;
    Gtk::ToggleButton expandEntriesBtn;
    Gtk::ToggleButton expandMeaningsBtn;

    std::vector<TextDrawPtr> textDrawContainer;
    std::span<TextDrawPtr> textDrawPronounciations;
    std::span<TextDrawPtr> textDrawMeaning_multipleEntries;
    std::span<TextDrawPtr> textDrawMeanings_singleEntry;

    std::vector<ZH_Dictionary::Entry> entries;
    size_t choice_entry;

    int active = -1;
    std::function<void(std::optional<int> choice)> func_vocableChoice;
};
