#include <MainWindow.h>

#include <spdlog/spdlog.h>
#include <filesystem>
#include <string_view>

constexpr std::string_view css =
    ".overlay {"
    "background-color: rgb(32,32,32);"
    "border: 1px solid white;"
    "}";

MainWindow::MainWindow()
    : m_VBox(Gtk::Orientation::VERTICAL), box_vocabulary(Gtk::Orientation::VERTICAL) {
    set_default_size(1920, 1080);
    set_title("Zikhron");
    Gtk::Overlay* ov = new Gtk::Overlay();
    ov->set_child(sidebar);
    set_child(*ov);
    sidebar.set_tab_pos(Gtk::PositionType::LEFT);

    sidebar.show();
    label_cards.show();

    label_cards.set_text("cards");
    label_vocabulary.set_text("vocabulary");
    displayCard = std::make_unique<DisplayCard>(*ov);

    sidebar.append_page(*displayCard, label_cards);
    sidebar.append_page(m_VBox, label_vocabulary);
    refCssProvider = Gtk::CssProvider::create();
    refCssProvider->load_from_data(std::string{css});
    auto screen = Gdk::Display::get_default();

    Gtk::StyleContext::add_provider_for_display(
        screen, refCssProvider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

MainWindow::~MainWindow() { DataThread::destroy(); }
