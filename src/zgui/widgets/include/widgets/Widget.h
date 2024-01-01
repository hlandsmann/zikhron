#pragma once
#include <cstddef>
#include <context/Theme.h>
#include <context/WidgetIdGenerator.h>
#include <fmt/format.h>
#include <imgui.h>

#include <magic_enum.hpp>
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

class Widget;

struct WidgetInit
{
    std::shared_ptr<context::Theme> theme;
    std::shared_ptr<context::WidgetIdGenerator> widgetIdGenerator;
    std::shared_ptr<layout::Rect> rect;
    layout::Orientation orientation;
    layout::Align align;
    std::weak_ptr<Widget> parent;
};

class Widget : public std::enable_shared_from_this<Widget>
{
public:
    Widget(WidgetInit init);
    virtual ~Widget() = default;
    Widget(const Widget&) = default;
    Widget(Widget&&) = default;
    auto operator=(const Widget&) -> Widget& = default;
    auto operator=(Widget&&) -> Widget& = default;

    /* Arrange
     * return true if (re)arrange  is necessary
     */
    virtual auto arrange() -> bool { return false; };
    [[nodiscard]] auto arrangeIsNecessary() -> bool;
    void setArrangeIsNecessary();
    [[nodiscard]] auto getTheme() const -> const context::Theme&;
    [[nodiscard]] auto getWidgetId() const -> int;
    [[nodiscard]] auto PassiveOrientation() const -> layout::Orientation;
    [[nodiscard]] auto Align() const -> layout::Align;
    [[nodiscard]] auto getWidgetSize() const -> const WidgetSize&;
    void resetWidgetSize();

protected:
    [[nodiscard]] virtual auto calculateSize() const -> WidgetSize = 0;
    [[nodiscard]] auto getThemePtr() const -> std::shared_ptr<context::Theme>;
    [[nodiscard]] auto getWidgetIdGenerator() const -> std::shared_ptr<context::WidgetIdGenerator>;
    [[nodiscard]] auto Rect() const -> const layout::Rect&;
    [[nodiscard]] auto getRectPtr() const -> std::shared_ptr<layout::Rect>;

private:
    std::shared_ptr<context::Theme> theme;
    std::shared_ptr<context::WidgetIdGenerator> widgetIdGenerator;
    std::shared_ptr<layout::Rect> rectPtr;
    layout::Orientation passiveOrientation;
    layout::Align baseAlign;
    std::weak_ptr<Widget> parent;
    mutable std::optional<WidgetSize> optWidgetSize;
    bool arrangeNecessary{true};
    int widgetId;
};

} // namespace widget

template<>
struct fmt::formatter<widget::layout::Align>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }
    template<typename FormatContext>
    auto format(widget::layout::Align align, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(), "{}", magic_enum::enum_name(align));
    }
};
template<>
struct fmt::formatter<widget::layout::SizeType>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }
    template<typename FormatContext>
    auto format(widget::layout::SizeType sizeType, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(), "{}", magic_enum::enum_name(sizeType));
    }
};
template<>
struct fmt::formatter<widget::layout::Orientation>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }
    template<typename FormatContext>
    auto format(widget::layout::Orientation orientation, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(), "{}", magic_enum::enum_name(orientation));
    }
};
