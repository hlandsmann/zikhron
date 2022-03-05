#include <MainWindow.h>

#include <spdlog/spdlog.h>
#include <filesystem>

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
}

MainWindow::~MainWindow() { DataThread::destroy(); }
