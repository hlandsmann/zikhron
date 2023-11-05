#pragma once
#include <imgui.h>

#include <optional>

namespace widget {
enum class size_type {
    fixed,
    variable
};

struct WidgetSize
{
    size_type widthType{size_type::fixed};
    size_type heightType{size_type::fixed};
    float width{};
    float height{};
};

class WidgetBase
{
public:
    WidgetBase() = default;
    virtual ~WidgetBase() = default;
    WidgetBase(const WidgetBase&) = default;
    WidgetBase(WidgetBase&&) = default;
    auto operator=(const WidgetBase&) -> WidgetBase& = default;
    auto operator=(WidgetBase&&) -> WidgetBase& = default;

    [[nodiscard]] virtual auto LayoutSize() const -> const WidgetSize& = 0;
};

template<class WidgetImpl>
class Widget : public WidgetBase
{
public:
    Widget() = default;
    ~Widget() override = default;
    Widget(const Widget&) = default;
    Widget(Widget&&) = default;
    auto operator=(const Widget&) -> Widget& = default;
    auto operator=(Widget&&) -> Widget& = default;

    [[nodiscard]] auto LayoutSize() const -> const WidgetSize& override
    {
        if (size.has_value()) {
            return *size;
        }
        return size.emplace(static_cast<WidgetImpl const*>(this)->calculateSize());
    }

private:
    mutable std::optional<WidgetSize> size;
};

} // namespace widget
