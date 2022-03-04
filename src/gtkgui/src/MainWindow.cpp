#include <MainWindow.h>

#include <spdlog/spdlog.h>
#include <filesystem>

MainWindow::MainWindow()
    : m_VBox(Gtk::Orientation::VERTICAL), box_vocabulary(Gtk::Orientation::VERTICAL) {
    set_default_size(1920, 1080);
    set_child(sidebar);

    sidebar.set_tab_pos(Gtk::PositionType::LEFT);
    // sidebar.append_page(tab, "text");

    sidebar.show();
    label_cards.show();

    label_cards.set_text("cards");
    label_vocabulary.set_text("vocabulary");
    displayCard = std::make_unique<DisplayCard>();
    data = std::make_unique<DataThread>(
        [this](auto&& msg_paragraph) { displayCard->receive_paragraph(std::move(msg_paragraph)); });
    displayCard->signal_submitEase(
        [this](const VocabularySR::Id_Ease_vt& id_ease) { data->submitEase(id_ease); });
    displayCard->signal_requestCard([this]() { data->requestCard(); });

    sidebar.append_page(*displayCard, label_cards);
    sidebar.append_page(m_VBox, label_vocabulary);
}
