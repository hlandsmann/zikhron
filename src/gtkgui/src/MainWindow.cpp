#include <MainWindow.h>

#include <DisplayCard.h>
#include <NotebookPage.h>
#include <VideoSpace.h>
#include <spdlog/spdlog.h>
#include <filesystem>
#include <string_view>
#include <type_traits>
#include <SupplyAudio.h>

constexpr std::string_view css =
    ".overlay {"
    "background-color: rgb(32,32,32);"
    "border: 1px solid white;"
    "}";

MainWindow::MainWindow() {
    std::setlocale(LC_NUMERIC, "C");
    set_default_size(1, 1);
    set_title("Zikhron");
    overlay = std::make_shared<Gtk::Overlay>();
    overlay->set_child(sidebar);
    set_child(*overlay);
    sidebar.set_tab_pos(Gtk::PositionType::LEFT);

    sidebar.show();

    appendPage(std::make_shared<DisplayCard>(*overlay), "Cards");
    appendPage(std::make_shared<VideoSpace>(*overlay), "Video");
    appendPage(std::make_shared<SupplyAudio>(), "Audio");

    sidebar.signal_switch_page().connect([this](Gtk::Widget *, guint slot) {
        for (guint page = 0; page < pages.size(); page++) {
            if (NotebookPage *notebookPage = dynamic_cast<NotebookPage *>(pages[page].get());
                notebookPage != nullptr)
                notebookPage->switchPage(slot == page);
        }
    });
    sidebar.set_current_page(DataThread::get().zikhronCfg.cfgMain.activePage);

    refCssProvider = Gtk::CssProvider::create();
    refCssProvider->load_from_data(std::string{css});
    auto screen = Gdk::Display::get_default();

    Gtk::StyleContext::add_provider_for_display(
        screen, refCssProvider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

MainWindow::~MainWindow() {
    DataThread::get().zikhronCfg.cfgMain.activePage = sidebar.get_current_page();
    DataThread::destroy();
}

template <class WidgetType>
void MainWindow::appendPage(const Glib::RefPtr<WidgetType> &page, const std::string &label) {
    static_assert(std::is_base_of_v<NotebookPage, WidgetType>,
                  "WidgetType needs to inherit from NotebookPage");
    pages.push_back(page);
    pageLabels.push_back(std::make_shared<Gtk::Label>(label));
    sidebar.append_page(*page, *pageLabels.back());
}
