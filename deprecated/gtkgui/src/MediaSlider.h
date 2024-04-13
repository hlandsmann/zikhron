#pragma once
#include <gtkmm.h>
#include <functional>
class MediaSlider : public Gtk::DrawingArea {
    constexpr static std::tuple<double, double, double> defaultColor = {0.8, 0.8, 0.8};
    std::tuple<double, double, double> drawColor = defaultColor;

public:
    MediaSlider();
    virtual ~MediaSlider() = default;
    void setSpacing(int spacing);
    void signal_clickProgress_connect(std::function<void(double)> fun) { signal_clickedProgress = fun; }
    void setProgress(double progress);
    auto getProgress() const -> double { return progress; }

protected:
    void on_draw(const Cairo::RefPtr<Cairo::Context> &cr, int width, int height);

private:
    int spacing = 5;
    double progress = 1.0;
    Glib::RefPtr<Gtk::GestureClick> clickController;
    std::function<void(double)> signal_clickedProgress;

    double lastWidth{};
    double lastHeight{};
};