#include <SubtitleOverlay.h>
#include <GTKMM_includes.h>

SubtitleOverlay::SubtitleOverlay() {
    testBtn.set_label("hello world");
    // textDraw.setText("之前");
    textDraw.setText("hello world");
    append(fixed);
    fixed.put(box, 0, 0);
    box.append(textDraw);
    // textDraw.get_style_context()->add_class("suboverlay");
    set_halign(Gtk::Align::END);
    set_valign(Gtk::Align::START);
    // box.set_halign(Gtk::Align::CENTER);
    // box.set_valign(Gtk::Align::CENTER);
    set_size_request(219, 60);
}
