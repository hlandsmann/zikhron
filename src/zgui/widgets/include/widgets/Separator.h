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

    Separator(const Separator&) = delete;
    Separator(Separator&&) = delete;
    auto operator=(const Separator&) -> Separator& = delete;
    auto operator=(Separator&&) -> Separator& = delete;

protected:
    auto calculateSize() const -> WidgetSize override;

private:
    WidgetSize widgetSize{};
};

}; // namespace widget
