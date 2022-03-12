#pragma once
#include <annotation/Ease.h>
#include <gtkmm.h>
#include <memory>
#include <string>
#include <vector>

class EaseChoice : public Gtk::Box {
public:
    EaseChoice();
    void setEase(Ease ease);
    auto getEase() const -> Ease;

private:
    std::vector<std::unique_ptr<Gtk::ToggleButton>> easeButtons;
    Ease ease{Ease::again};
};
