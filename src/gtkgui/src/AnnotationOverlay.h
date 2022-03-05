#pragma once

#include <TextDraw.h>
#include <gtkmm.h>
#include <functional>
#include <limits>
#include <optional>
#include <span>
#include <string>
#include <vector>

struct AnnotationOverlayInit {
    const std::string& activeChoice;
    const std::vector<std::string>& marked;
    const std::vector<std::string>& unmarked;
    int x;
    int y;
    int x_max;
    int y_max;
};

class AnnotationOverlay : public Gtk::Fixed {
public:
    using TextDrawPtr = std::unique_ptr<TextDraw>;
    AnnotationOverlay(AnnotationOverlayInit& init);
    void signal_annotationChoice(const std::function<void(std::optional<int> choice)>& functor) {
        func_annotationChoice = functor;
    };

private:
    void setupTextDraw();
    void show(int x, int y, int x_max, int y_max);
    void callback_motion(int index);
    void callback_leave(int index);

    constexpr static int fontSize = 40;

    Glib::RefPtr<Gtk::GestureClick> clickController;
    Gtk::Box box;
    std::vector<TextDrawPtr> textDrawContainer;
    std::span<TextDrawPtr> textDrawAlternativeView;

    std::vector<std::string> marked;
    std::vector<std::string> unmarked;
    const std::string activeChoice;

    int active = -1;
    std::function<void(std::optional<int> choice)> func_annotationChoice;
};
