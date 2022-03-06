#pragma once

#include <TextDraw.h>
#include <gtkmm.h>
#include <functional>
#include <optional>

struct VocableOverlayInit {
    const std::string& key;
    const std::string& pronounciationChoice;
    const std::string& meaningChoice;
    const std::vector<std::string>& pronounciations;
    const std::vector<std::string>& meanings;
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
    void setupTextDraw();
    void show(int x, int y, int x_max, int y_max);
    void callback_motion(int index);
    void callback_leave(int index);

    Glib::RefPtr<Gtk::GestureClick> clickController;
    Gtk::Box box;
    Gtk::Grid currentGrid;
    Gtk::Grid choiceGrid;
    Gtk::ToggleButton expandBtn;

    std::vector<TextDrawPtr> textDrawContainer;

    std::function<void(std::optional<int> choice)> func_vocableChoice;
};
