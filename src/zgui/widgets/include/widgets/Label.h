#pragma once
#include "detail/Widget.h" // IWYU pragma: export detail/Widget.h

#include <string>

namespace widget {

class Label : public Widget
{
    template<class T>
    friend class MetaBox;
    friend class Widget;
    void setup(std::string text);

public:
    Label(WidgetInit init);
    ~Label() override = default;

    Label(const Label&) = delete;
    Label(Label&&) = delete;
    auto operator=(const Label&) -> Label& = delete;
    auto operator=(Label&&) -> Label& = delete;

protected:
    auto calculateSize() const -> WidgetSize override;

private:
    std::string text;
};
} // namespace widget
