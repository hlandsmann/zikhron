#pragma once

#include <TextDraw.h>
#include <gtkmm.h>

class SubtitleOverlay : public Gtk::Box {
public:
    SubtitleOverlay();

private:
    Gtk::Box box;
    Gtk::Fixed fixed;
    Gtk::Button testBtn;
    TextDraw textDraw;
};