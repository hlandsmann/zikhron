#include <TextDraw.h>
#include <cairomm/context.h>
#include <spdlog/spdlog.h>
#include <functional>

TextDraw::TextDraw() {
    set_draw_func(sigc::mem_fun(*this, &TextDraw::on_draw));
    setFontSize(defaultFontSize);
    motionController = Gtk::EventControllerMotion::create();
    motionController->signal_motion().connect(
        [this](double x, double y) { signal_mouseMotion(int(x), int(y)); });
    motionController->signal_leave().connect([this]() { signal_mouseLeave(); });
    motionController->set_propagation_phase(Gtk::PropagationPhase::CAPTURE);
    add_controller(motionController);

    clickController = Gtk::GestureClick::create();
    clickController->signal_pressed().connect([this](int key, double x, double y) {
        if (key == 1)
            signal_mouseClick(int(x), int(y));
    });
    add_controller(clickController);
}

TextDraw::~TextDraw() {}

void TextDraw::on_draw(const Cairo::RefPtr<Cairo::Context> &cr, int width, int height) {
    // spdlog::info("OnDraw, width: {}, height: {}", width, height);
    std::apply([&](auto... color) { cr->set_source_rgb(color...); }, fontColor);

    if (drawBorder) {
        cr->set_line_width(1.0);
        cr->rectangle(0, 0, width, height);
        cr->stroke();
    }
    cr->move_to(spacing, spacing + drift);

    auto layout = freshLayout();
    layout->set_width(sizeNoSpace(width) * Pango::SCALE);
    layout->show_in_cairo_context(cr);
    lastDrawnLayout = layout;

    newHeightRequest(height, layout);
}

void TextDraw::newHeightRequest(int height, Glib::RefPtr<Pango::Layout> &layout) {
    int width, neededHeight;
    layout->get_pixel_size(width, neededHeight);

    drift = calculateDrift(layout);
    neededHeight = std::max(sizeWithSpace(neededHeight) + drift, sizeWithSpace(neededHeight));

    if (neededHeight == height || lastNeededHeight == neededHeight)
        return;
    lastNeededHeight = neededHeight;

    // spdlog::warn("fs: {}, drift: {}, height: {}", fontSize, drift, neededHeight);

    currentNaturalHeight = neededHeight;
    if (currentMinHeight > currentNaturalHeight)
        currentMinHeight = currentNaturalHeight;
    if (hard_size_request)
        set_size_request(-1, neededHeight);
    queue_resize();
}

auto TextDraw::calculateDrift(Glib::RefPtr<Pango::Layout> &layout) -> int {
    int baseline = (layout->get_baseline() / Pango::SCALE);
    int _drift = fontSize - baseline;
    // spdlog::info("bl: {}, fs: {}", baseline, fontSize);
    _drift = std::clamp(_drift, -spacing, spacing);
    return _drift;
}

void TextDraw::setText(const std::string &str) {
    text = str;
    std::tie(currentNaturalWidth, currentMinHeight) = calculateOrientationHorizontal();
    std::tie(currentMinWidth, currentNaturalHeight) = calculateOrientationVertical();
    currentNaturalHeight = currentMinHeight;  // prevent flickering
    queue_resize();
}

void TextDraw::setFontSize(int _fontSize) {
    fontSize = _fontSize;
    font.set_size(fontSize * Pango::SCALE);
    // setText(text);
    queue_draw();
}

void TextDraw::setFontColor(double r, double g, double b) {
    fontColor = {r, g, b};
    queue_draw();
}

void TextDraw::setFontColorDefault() {
    std::apply([this](auto... color) { setFontColor(color...); }, defaultFontColor);
}

void TextDraw::setSpacing(int _spacing) {
    spacing = _spacing;
    // setText(text);
    queue_draw();
}

void TextDraw::setDrawBorder(bool drawBorder_in) {
    drawBorder = drawBorder_in;
    queue_draw();
}

auto TextDraw::getCharacterPosition(int byteIndex) -> Gdk::Rectangle {
    auto pangoRectangoe = lastDrawnLayout->index_to_pos(byteIndex);
    Gdk::Rectangle resultRectangle;
    resultRectangle.set_x((pangoRectangoe.get_x() / Pango::SCALE) + spacing);
    resultRectangle.set_y((pangoRectangoe.get_y() / Pango::SCALE) + spacing + drift);
    resultRectangle.set_width(pangoRectangoe.get_width() / Pango::SCALE);
    resultRectangle.set_height(pangoRectangoe.get_height() / Pango::SCALE);

    double x, y;
    Gtk::Widget *root = dynamic_cast<Gtk::Widget *>(get_root());
    translate_coordinates(*root, double(resultRectangle.get_x()), double(resultRectangle.get_y()), x, y);
    resultRectangle.set_x(int(x));
    resultRectangle.set_y(int(y));
    return resultRectangle;
}

Glib::RefPtr<Pango::Layout> TextDraw::freshLayout() {
    Glib::RefPtr<Pango::Layout> layout = create_pango_layout("");
    layout->set_font_description(font);
    layout->set_wrap(Pango::WrapMode::WORD);
    layout->set_markup(text);
    return layout;
}

auto TextDraw::calculateOrientationHorizontal() -> std::pair<int, int> {
    int width, height;
    auto layout = freshLayout();
    drift = calculateDrift(layout);
    layout->get_pixel_size(width, height);
    auto naturalWidth = sizeWithSpace(width);
    auto minHeight = std::max(sizeWithSpace(height) + drift, sizeWithSpace(height));
    return {naturalWidth, minHeight};
}

auto TextDraw::calculateOrientationVertical() -> std::pair<int, int> {
    int width, height;
    auto layout = freshLayout();
    drift = calculateDrift(layout);
    layout->set_width(0);
    layout->get_pixel_size(width, height);
    auto minWidth = sizeWithSpace(width);
    auto naturalHeight = std::max(sizeWithSpace(height) + drift, sizeWithSpace(height));
    return {minWidth, naturalHeight};
}

void TextDraw::update_markup(const std::string &markup) {
    text = markup;
    queue_draw();
}

void TextDraw::measure_vfunc(Gtk::Orientation orientation,
                             int /*for_size*/,
                             int &minimum,
                             int &natural,
                             int &,
                             int &) const {
    switch (orientation) {
    case Gtk::Orientation::HORIZONTAL:
        minimum = currentMinWidth;
        natural = currentNaturalWidth;
        break;
    case Gtk::Orientation::VERTICAL:
        minimum = currentMinHeight;
        natural = currentNaturalHeight;
        break;
    }
    // spdlog::info("Orientation: {}, f{}, m{}", int(orientation), for_size, natural);
}

void TextDraw::signal_mouseMotion(int x, int y) {
    auto byteIndex = mousePosToByteIndex(x, y);

    if (func_hoverByteIndex && lastByteIndex != byteIndex) {
        func_hoverByteIndex(byteIndex);
        lastByteIndex = byteIndex;
        // spdlog::info("index: {}", byteIndex);
    }

    // spdlog::info("MouseX: {}, MouseY: {}, index: {}, trailing: {}", x, y, byteIndex, trailing);
}

void TextDraw::signal_mouseClick(int x, int y) {
    auto byteIndex = mousePosToByteIndex(x, y);
    if (func_clickByteIndex) {
        func_clickByteIndex(byteIndex);
        lastByteIndex = byteIndex;
    }
    // spdlog::info("MouseX: {}, MouseY: {}, index: {}, trailing: {}", x, y, byteIndex, trailing);
}

void TextDraw::signal_mouseLeave() {
    if (func_mouseLeave)
        func_mouseLeave();
    lastByteIndex = -1;
}

auto TextDraw::mousePosToByteIndex(int x, int y) const -> int {
    auto layout = lastDrawnLayout;
    if (layout == nullptr)
        return -1;
    int byteIndex, trailing;
    int width, height;
    layout->get_pixel_size(width, height);
    x -= spacing;
    y -= (spacing + drift);

    if (x < 0 || y < 0 || x > width || y > height)
        return -1;
    layout->xy_to_index(x * Pango::SCALE, y * Pango::SCALE, byteIndex, trailing);
    auto scaled_x = x * Pango::SCALE;
    auto scaled_y = y * Pango::SCALE;
    auto rect = layout->index_to_pos(byteIndex);
    if (rect.get_x() > scaled_x || rect.get_x() + rect.get_width() < scaled_x ||
        rect.get_y() > scaled_y || rect.get_y() + rect.get_height() < scaled_y)
        return -1;

    return byteIndex;
}

auto TextDraw::getWidth() const -> int { return currentNaturalWidth; }

auto TextDraw::getHeight() const -> int { return currentNaturalHeight; }
