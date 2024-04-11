#pragma once
#include "detail/Widget.h" // IWYU pragma: export detail/Widget.h

#include <context/Theme.h>
#include <imgui.h>

#include <string>

namespace widget {

class Button : public Widget
{
    template<class T>
    friend class MetaBox;
    friend class Widget;
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

protected:
    auto calculateSize() const -> WidgetSize override;

private:
    static constexpr float buttonPadding = 4;

    bool sensitive{true};
    bool checked{false};

    std::string label;
};

}; // namespace widget
