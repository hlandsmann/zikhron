#pragma once
#include <context/Theme.h>
#include <imgui.h>

#include <memory>
#include <optional>
#include <utility>

namespace widget {
namespace layout {
struct Rect
{
    auto operator==(Rect const&) const -> bool = default;
    float x{};
    float y{};
    float width{};
    float height{};
};

enum class Orientation {
    horizontal,
    vertical
};

enum class Align {
    start,
    center,
    end
};
enum class SizeType {
    fixed,
    width_fixed = fixed,
    height_fixed = fixed,
    expand,
    width_expand = expand,
    height_expand = expand,
};
static auto constexpr width_fixed = SizeType::width_fixed;
static auto constexpr height_fixed = SizeType::height_fixed;
static auto constexpr width_expand = SizeType::width_expand;
static auto constexpr height_expand = SizeType::height_expand;
} // namespace layout

struct WidgetSize
{
    using SizeType = layout::SizeType;
    SizeType widthType{SizeType::fixed};
    SizeType heightType{SizeType::fixed};
    float width{};
    float height{};
};

struct WidgetInit
{
    std::shared_ptr<context::Theme> theme;
    std::shared_ptr<layout::Rect> rect;
    layout::Orientation orientation;
    layout::Align align;
};

class WidgetBase
{
public:
    WidgetBase(WidgetInit init);
    virtual ~WidgetBase() = default;
    WidgetBase(const WidgetBase&) = default;
    WidgetBase(WidgetBase&&) = default;
    auto operator=(const WidgetBase&) -> WidgetBase& = default;
    auto operator=(WidgetBase&&) -> WidgetBase& = default;

    virtual auto arrange() -> bool { return true; };
    [[nodiscard]] virtual auto getWidgetSize() const -> const WidgetSize& = 0;
    [[nodiscard]] auto getTheme() const -> const context::Theme&;
    [[nodiscard]] auto PassiveOrientation() const -> layout::Orientation;
    [[nodiscard]] auto Align() const -> layout::Align;

protected:
    [[nodiscard]] auto getThemePtr() const -> std::shared_ptr<context::Theme>;
    [[nodiscard]] auto Rect() const -> const layout::Rect&;

private:
    std::shared_ptr<context::Theme> theme;
    std::shared_ptr<layout::Rect> rectPtr;
    layout::Orientation passiveOrientation;
    layout::Align baseAlign;
};

template<class WidgetImpl>
class Widget : public WidgetBase
{
public:
    Widget(WidgetInit init)
        : WidgetBase{std::move(init)} {}
    ~Widget() override = default;
    Widget(const Widget&) = default;
    Widget(Widget&&) = default;
    auto operator=(const Widget&) -> Widget& = default;
    auto operator=(Widget&&) -> Widget& = default;

    [[nodiscard]] auto getWidgetSize() const -> const WidgetSize& override
    {
        if (optWidgetSize.has_value()) {
            return *optWidgetSize;
        }
        return optWidgetSize.emplace(static_cast<WidgetImpl const*>(this)->calculateSize());
    }

    void resetWidgetSize()
    {
        optWidgetSize.reset();
    }

private:
    mutable std::optional<WidgetSize> optWidgetSize;
};

} // namespace widget
