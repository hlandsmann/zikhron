#pragma once

#include <DataThread.h>
#include <DisplayCard.h>
#include <dictionary/ZH_Dictionary.h>
#include <gtkmm.h>
#include <spaced_repetition/VocabularySR.h>
#include <memory>

class MainWindow : public Gtk::Window {
public:
    MainWindow();
    virtual ~MainWindow();

private:
    Gtk::Box m_VBox;
    Gtk::Box box_vocabulary;
    Gtk::Notebook sidebar;
    Gtk::Label label_cards;
    Gtk::Label label_vocabulary;
    std::unique_ptr<DisplayCard> displayCard;
};
