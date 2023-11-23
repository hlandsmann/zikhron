#pragma once

#include <DataThread.h>
#include <dictionary/ZH_Dictionary.h>
#include <gtkmm.h>
#include <memory>
#include <vector>

class MainWindow : public Gtk::Window {
public:
    MainWindow();
    ~MainWindow() override;

private:
    template <class WidgetType>
    void appendPage(const Glib::RefPtr<WidgetType>& page, const std::string& label);

    Glib::RefPtr<Gtk::CssProvider> refCssProvider;
    std::shared_ptr<Gtk::Overlay> overlay;

    std::vector<Glib::RefPtr<Gtk::Widget>> pages;
    std::vector<Glib::RefPtr<Gtk::Label>> pageLabels;

    // sidebar callback triggers on destruction - used data members may be invalid and lead to a crash.
    // Hence destroy it first!
    Gtk::Notebook sidebar;
};
