#pragma once
#include <context/Theme.h>
#include <context/WidgetIdGenerator.h>
#include <fmt/core.h>
#include <fmt/format.h> // IWYU pragma: export core.h
#include <imgui.h>

#include <cstddef>
#include <format>
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
enum class ExpandType {
    fixed,
    width_fixed = fixed,
    height_fixed = fixed,
    expand,
    width_expand = expand,
    height_expand = expand,
};
static auto constexpr width_fixed = ExpandType::width_fixed;
static auto constexpr height_fixed = ExpandType::height_fixed;
static auto constexpr width_expand = ExpandType::width_expand;
static auto constexpr height_expand = ExpandType::height_expand;
} // namespace layout

struct WidgetSize
{
    using ExpandType = layout::ExpandType;
    ExpandType widthType{ExpandType::fixed};
    ExpandType heightType{ExpandType::fixed};
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
    layout::Align horizontalAlign;
    layout::Align verticalAlign;
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

    template<class WidgetType, class... Args>
    auto create(Args... args) -> std::shared_ptr<WidgetType>
    {
        auto init = makeWidgetInit();

        auto widget = std::make_shared<WidgetType>(std::move(init));
        widget->setup(std::forward<Args>(args)...);
        return widget;
    }

    /* reset funktions for arange and size calculation will not propagate to ancestors */
    template<class WidgetType, class... Args>
    auto createOrphan(Args... args) -> std::shared_ptr<WidgetType>
    {
        auto init = makeWidgetInit();
        init.parent = std::weak_ptr<Widget>{};

        auto widget = std::make_shared<WidgetType>(std::move(init));
        widget->setup(std::forward<Args>(args)...);
        return widget;
    }

    /* Arrange
     * return true if (re)arrange  is necessary
     */
    virtual auto arrange() -> bool { return false; };
    [[nodiscard]] auto arrangeIsNecessary() -> bool;
    void setArrangeIsNecessary();
    [[nodiscard]] auto getTheme() const -> const context::Theme&;
    [[nodiscard]] auto getWidgetId() const -> int;
    [[nodiscard]] auto dropWidgetId() const -> context::WidgetIdDrop;
    [[nodiscard]] auto PassiveOrientation() const -> layout::Orientation;
    [[nodiscard]] auto HorizontalAlign() const -> layout::Align;
    void setHorizontalAlign(layout::Align);
    [[nodiscard]] auto VerticalAlign() const -> layout::Align;
    void setVerticalAlign(layout::Align);
    [[nodiscard]] auto getWidgetSize() const -> const WidgetSize&;
    [[nodiscard]] auto getWidgetMinSize() const -> const WidgetSize&;
    void resetWidgetSize();

protected:
    [[nodiscard]] virtual auto calculateSize() const -> WidgetSize = 0;
    [[nodiscard]] virtual auto calculateMinSize() const -> WidgetSize;
    [[nodiscard]] auto getThemePtr() const -> std::shared_ptr<context::Theme>;
    [[nodiscard]] auto getWidgetIdGenerator() const -> std::shared_ptr<context::WidgetIdGenerator>;
    [[nodiscard]] auto Rect() const -> const layout::Rect&;
    [[nodiscard]] auto getRectPtr() const -> std::shared_ptr<layout::Rect>;

private:
    auto makeWidgetInit() -> WidgetInit;
    std::shared_ptr<context::Theme> theme;
    std::shared_ptr<context::WidgetIdGenerator> widgetIdGenerator;
    std::shared_ptr<layout::Rect> rectPtr;
    layout::Orientation passiveOrientation;
    layout::Align horizontalAlign;
    layout::Align verticalAlign;
    std::weak_ptr<Widget> parent;
    mutable std::optional<WidgetSize> optWidgetSize;
    mutable std::optional<WidgetSize> optWidgetMinSize;
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
struct fmt::formatter<widget::layout::ExpandType>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }
    template<typename FormatContext>
    auto format(widget::layout::ExpandType expandType, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(), "{}", magic_enum::enum_name(expandType));
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
