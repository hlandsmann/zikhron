#pragma once
#include <gtkmm.h>
#include <spdlog/spdlog.h>

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
    void set_hard_size_request(bool request = true) { hard_size_request = request; };
    void setFontSize(int fontSize);
    void setFontColor(double r, double g, double b);
    void setFontColorDefault();
    void setSpacing(int spacing);
    void setDrawBorder(bool drawBorder = true);
    auto getCharacterPosition(int byteIndex) -> Gdk::Rectangle;
    auto getWidth() const -> int;
    auto getHeight() const -> int;
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
    int fontSize = defaultFontSize;

    constexpr static std::tuple<double, double, double> defaultFontColor = {0.8, 0.8, 0.8};
    std::tuple<double, double, double> fontColor = defaultFontColor;

    std::string text = "no text";
    int spacing = 30;
    int drift = 0;
    int currentMinWidth, currentNaturalWidth;
    int currentMinHeight, currentNaturalHeight;

    Pango::FontDescription font;
    std::function<void(int)> func_hoverByteIndex;
    std::function<void(int)> func_clickByteIndex;
    std::function<void()> func_mouseLeave;
    int lastByteIndex = -1;
    bool drawBorder = false;
    bool hard_size_request = false;
};
