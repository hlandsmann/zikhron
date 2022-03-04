#pragma once
#include <gtkmm.h>

class TextDraw : public Gtk::DrawingArea {
public:
    TextDraw();
    void setText(const std::string &str);
    void update_markup(const std::string &markup);
    virtual ~TextDraw();
    void measure_vfunc(Gtk::Orientation orientation,
                       int for_size,
                       int &minimum,
                       int &natural,
                       int &minimum_baseline,
                       int &natural_baseline) const override;
    void setFontSize(int fontSize);
    void setSpacing(int spacing);

    void signal_hoverByteIndex(const std::function<void(int byteIndex)> &functor) {
        func_hoverByteIndex = functor;
    };
    void signal_mouseClickByteIndex(const std::function<void(int byteIndex)> &functor) {
        func_clickByteIndex = functor;
    };
    void signal_mouseLeave(const std::function<void()> &functor) { func_mouseLeave = functor; };

protected:
    void on_draw(const Cairo::RefPtr<Cairo::Context> &cr, int width, int height);

private:
    // int heightForWidth(int width) const;
    // int widthForHeight(int height) const;
    Glib::RefPtr<Pango::Layout> freshLayout();
    auto sizeWithSpace(int width_or_height) -> int { return width_or_height + (spacing); }
    auto sizeNoSpace(int width_or_height) -> int { return width_or_height - (spacing); }
    auto calculateOrientationHorizontal() -> std::pair<int, int>;
    auto calculateOrientationVertical() -> std::pair<int, int>;
    void newHeightRequest(int height, Glib::RefPtr<Pango::Layout> &layout);
    void signal_mouseMotion(int x, int y);
    void signal_mouseClick(int x, int y);
    void signal_mouseLeave();
    auto mousePosToByteIndex(int x, int y) const -> int;
    auto calculateDrift(Glib::RefPtr<Pango::Layout> &layout) -> int;

    Glib::RefPtr<Gtk::EventControllerMotion> motionController;
    Glib::RefPtr<Gtk::GestureClick> clickController;

    int lastNeededHeight = 0;
    Glib::RefPtr<Pango::Layout> lastDrawnLayout = nullptr;

    constexpr static int defaultFontSize = 40;
    std::string text = "no text";
    int fontSize = 40;
    int spacing = 30;
    int drift = 0;
    int currentMinWidth, currentNaturalWidth;
    int currentMinHeight, currentNaturalHeight;

    int currentHeight = 0;
    int currentWidth = 0;

    // Glib::RefPtr<Pango::Layout> layout;
    Pango::FontDescription font;

    std::function<void(int)> func_hoverByteIndex;
    std::function<void(int)> func_clickByteIndex;
    std::function<void()> func_mouseLeave;
    int lastByteIndex = -1;
};
