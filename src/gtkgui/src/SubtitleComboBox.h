#pragma once
#include <gtkmm.h>

struct SubtitleRecord : public Gtk::TreeModel::ColumnRecord {
    SubtitleRecord();

    Gtk::TreeModelColumn<int> id;
    Gtk::TreeModelColumn<Glib::ustring> languageCode;
    Gtk::TreeModelColumn<Glib::ustring> description;
};

class SubtitleComboBox : public Gtk::ComboBox {
public:
    SubtitleComboBox();

private:
    SubtitleRecord subtitleRecord;
    Glib::RefPtr<Gtk::ListStore> listStore;

};