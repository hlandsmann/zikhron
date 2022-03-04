#pragma once

#include <DisplayCard.h>
#include <dictionary/ZH_Dictionary.h>
#include <gtkmm.h>
#include <spaced_repetition/VocabularySR.h>
#include <DataThread.h>
#include <memory>

// #include <gtkmm/application.h>

class MainWindow : public Gtk::Window {
public:
    MainWindow();
    virtual ~MainWindow() = default;

private:

    Gtk::Box m_VBox;
    Gtk::Box box_vocabulary;
    Gtk::Notebook sidebar;
    Gtk::Label label_cards;
    Gtk::Label label_vocabulary;
    std::unique_ptr<DisplayCard> displayCard;

    std::unique_ptr<DataThread> data;
};
