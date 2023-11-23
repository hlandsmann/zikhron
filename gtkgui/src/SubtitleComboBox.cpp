#include <SubtitleComboBox.h>

SubtitleRecord::SubtitleRecord() {
    add(id);
    add(languageCode);
    add(description);
}

SubtitleComboBox::SubtitleComboBox() {
    listStore = Gtk::ListStore::create(subtitleRecord);
    set_model(listStore);
    auto iter = listStore->append();
    auto row = *iter;
    row[subtitleRecord.id] = 0;
    row[subtitleRecord.languageCode] = "chi";
    row[subtitleRecord.description] = "simplified Chinese";
    set_active(iter);

    pack_start(subtitleRecord.id);
    pack_start(subtitleRecord.languageCode);
}