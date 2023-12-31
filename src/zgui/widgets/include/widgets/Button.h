#pragma once
#include "Widget.h"

#include <context/Theme.h>
#include <imgui.h>

#include <string>

namespace widget {

class Button : public Widget<Button>
{
  friend class Box;
  void setup(std::string label);
public:
    Button(WidgetInit init);
    ~Button() override = default;

    Button(const Button&) = default;
    Button(Button&&) = default;
    auto operator=(const Button&) -> Button& = default;
    auto operator=(Button&&) -> Button& = default;

    auto clicked() const -> bool;
    void setChecked(bool checked);
    void setSensitive(bool sensitive);

private:
    static constexpr float buttonPadding = 4;
    friend class Widget<Button>;
    auto calculateSize() const -> WidgetSize;

    bool sensitive{true};
    bool checked{false};

    std::string label;
};

}; // namespace widget
