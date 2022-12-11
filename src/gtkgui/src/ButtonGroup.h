#pragma once
#include <gtkmm.h>
#include <utils/Property.h>
#include <memory>
#include <string>
#include <vector>

class ButtonGroup : public Gtk::Box {
public:
    ButtonGroup(std::initializer_list<std::string> labels);
    template <class... T> ButtonGroup(T&&... t) : ButtonGroup({std::string(std::forward<T>(t))...}) {}

    void setActive(uint active);
    auto getActive() const -> uint;
    void observe_active(const std::function<void(uint)> observer);

private:
    utl::ObserverCollection observers;

    std::vector<std::unique_ptr<Gtk::ToggleButton>> buttons;
    utl::Property<uint> active;
};
