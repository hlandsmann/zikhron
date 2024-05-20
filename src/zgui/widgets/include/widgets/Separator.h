#pragma once
#include "detail/Widget.h" // IWYU pragma: export detail/Widget.h

namespace widget {

class Separator : public Widget
{
    template<class T>
    friend class MetaBox;
    friend class Widget;
    void setup();
    void setup(float width, float height);

public:
    Separator(WidgetInit init);
    ~Separator() override = default;

    Separator(const Separator&) = default;
    Separator(Separator&&) = default;
    auto operator=(const Separator&) -> Separator& = default;
    auto operator=(Separator&&) -> Separator& = default;

protected:
    auto calculateSize() const -> WidgetSize override;

private:
    WidgetSize widgetSize{};
};

}; // namespace widget
