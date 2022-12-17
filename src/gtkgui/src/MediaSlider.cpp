#include <MediaSlider.h>
#include <spdlog/spdlog.h>
#include <algorithm>
#include <functional>
MediaSlider::MediaSlider() {
    set_draw_func(sigc::mem_fun(*this, &MediaSlider::on_draw));
    clickController = Gtk::GestureClick::create();
    clickController->signal_pressed().connect([this](int key, double x, double y) {
        double totalBarLength = lastWidth - spacing * 4;
        double bar_x = spacing * 2;

        double clickedProgress = std::clamp(x - bar_x, 0., totalBarLength);
        if (signal_clickedProgress)
            signal_clickedProgress(std::clamp(clickedProgress / totalBarLength, 0., 1.));
    });
    add_controller(clickController);
}

void MediaSlider::on_draw(const Cairo::RefPtr<Cairo::Context> &cr, int width, int height) {
    lastWidth = width;
    lastHeight = height;
    std::apply([&](auto... color) { cr->set_source_rgb(color...); }, drawColor);

    double x0 = spacing;
    double y0 = spacing;
    cr->set_line_width(1.0);
    cr->rectangle(x0, y0, width - spacing * 2, height - spacing * 2);
    cr->stroke();

    double totalBarLength = width - spacing * 4;
    double bar_x = spacing * 2;
    double bar_y = height / 2;
    cr->move_to(bar_x, bar_y);
    cr->set_line_width(height - spacing * 4);
    cr->line_to(bar_x + totalBarLength * progress, bar_y);
    cr->stroke();

}

void MediaSlider::setSpacing(int _spacing) {
    spacing = _spacing;
    queue_draw();
}

void MediaSlider::setProgress(double _progress) {
    progress = _progress;
    queue_draw();
}
