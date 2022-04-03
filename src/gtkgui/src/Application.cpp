#include <Application.h>
#include <MainWindow.h>
#include <gtkmm.h>

int Application::run(int argc, char* argv[]) {
    auto app = Gtk::Application::create("org.gtkmm.zikhron");

    // Shows the window and returns when it is closed.
    return app->make_window_and_run<MainWindow>(argc, argv);
}
