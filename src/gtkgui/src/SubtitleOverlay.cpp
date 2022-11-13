#include <SubtitleOverlay.h>

SubtitleOverlay::SubtitleOverlay() {
    testBtn.set_label("hello world");
    textDraw.setText("Hello World");
    append(fixed);
    fixed.put(box, 0, 0);
    box.append(textDraw);
    set_halign(Gtk::Align::END);
    set_valign(Gtk::Align::START);
    // box.set_halign(Gtk::Align::CENTER);
    // box.set_valign(Gtk::Align::CENTER);
    set_size_request(50, 50);
}