#pragma once
#include <context/Theme.h>
#include <context/WidgetIdGenerator.h>
#include <context/imglog.h>
#include <imgui.h>
#include <utils/spdlog.h>

#include <format>
#include <magic_enum.hpp>
#include <memory>
#include <optional>
#include <string>
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

enum class Align {
    start,
    center,
    end
};
enum class ExpandType {
    fixed,
    width_fixed = fixed,
    height_fixed = fixed,
    adapt,
    width_adapt = adapt,
    height_adapt = adapt,
    expand,
    width_expand = expand,
    height_expand = expand,
};
static auto constexpr width_fixed = ExpandType::width_fixed;
static auto constexpr height_fixed = ExpandType::height_fixed;
static auto constexpr width_adapt = ExpandType::width_adapt;
static auto constexpr height_adapt = ExpandType::height_adapt;
static auto constexpr width_expand = ExpandType::width_expand;
static auto constexpr height_expand = ExpandType::height_expand;
} // namespace layout

struct WidgetSize
{
    float width{0.0F};
    float height{0.0F};
};

class Widget;

struct WidgetInit
{
    std::shared_ptr<context::Theme> theme;
    std::shared_ptr<context::WidgetIdGenerator> widgetIdGenerator;
    std::shared_ptr<layout::Rect> rect;
    layout::Align horizontalAlign;
    layout::Align verticalAlign;
    layout::ExpandType expandTypeWidth{layout::ExpandType::width_fixed};
    layout::ExpandType expandTypeHeight{layout::ExpandType::height_fixed};
    std::weak_ptr<Widget> parent;
};

class Widget : public std::enable_shared_from_this<Widget>
{
    using ExpandType = layout::ExpandType;

public:
    using WidgetId = context::WidgetId;
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
    virtual auto arrange(const layout::Rect& rect) -> bool;
    [[nodiscard]] auto arrangeIsNecessary() -> bool;
    void setArrangeIsNecessary();
    [[nodiscard]] auto getTheme() const -> const context::Theme&;
    [[nodiscard]] auto getWidgetId() const -> WidgetId;
    [[nodiscard]] auto dropWidgetId() const -> context::WidgetIdDrop;
    [[nodiscard]] auto HorizontalAlign() const -> layout::Align;
    void setHorizontalAlign(layout::Align);
    [[nodiscard]] auto VerticalAlign() const -> layout::Align;
    void setVerticalAlign(layout::Align);
    [[nodiscard]] auto getWidgetSize() const -> const WidgetSize&;
    [[nodiscard]] virtual auto getWidgetSizeFromRect(const layout::Rect&) -> WidgetSize;
    [[nodiscard]] auto getWidgetMinSize() const -> const WidgetSize&;
    void resetWidgetSize();
    void setName(const std::string& name);
    [[nodiscard]] auto getName() const -> const std::string&;

    void setExpandType(layout::ExpandType width, layout::ExpandType height);
    auto getExpandTypeWidth() const -> ExpandType;
    auto getExpandTypeHeight() const -> ExpandType;

    // debug functions
    template<class... Args>
    void winlog(const std::string& _name, std::format_string<Args...> fmt, Args&&... args) const
    {
        if (_name.empty() || _name == name) {
            imglog::log(fmt, std::forward<Args>(args)...);
        }
    }

    template<class... Args>
    void parentlog(const std::string& _name, std::format_string<Args...> fmt, Args&&... args) const
    {
        if (anyParentHasName(_name)) {
            imglog::log(fmt, std::forward<Args>(args)...);
        }
    }

    template<class... Args>
    void consoleLog(const std::string& _name, fmt::format_string<Args...> fmt, Args&&... args) const
    {
        if (_name.empty() || _name == name) {
            spdlog::info(fmt, std::forward<Args>(args)...);
        }
    }

    void cutWidgetIdGen();
    [[nodiscard]] auto getParent() const -> std::shared_ptr<Widget>;

    [[nodiscard]] auto anyParentHasName(const std::string& name) const -> bool;
    [[nodiscard]] auto anyParentHasId(WidgetId id) const -> bool;

    [[nodiscard]] auto anyParentHasId(unsigned id) const -> bool { return anyParentHasId(static_cast<WidgetId>(id)); }
    void scratchDbg();

protected:
    [[nodiscard]] static auto dropWidgetId(WidgetId) -> context::WidgetIdDrop;
    [[nodiscard]] virtual auto calculateSize() const -> WidgetSize = 0;
    [[nodiscard]] virtual auto calculateMinSize() const -> WidgetSize;
    [[nodiscard]] auto getThemePtr() const -> std::shared_ptr<context::Theme>;
    [[nodiscard]] auto getWidgetIdGenerator() const -> std::shared_ptr<context::WidgetIdGenerator>;
    [[nodiscard]] auto getRect() const -> const layout::Rect&;
    void setRect(const layout::Rect&);

private:
    auto makeWidgetInit() -> WidgetInit;
    std::shared_ptr<context::Theme> theme;
    std::shared_ptr<context::WidgetIdGenerator> widgetIdGenerator;
    mutable std::string name;
    std::shared_ptr<layout::Rect> rectPtr;
    layout::Align horizontalAlign;
    layout::Align verticalAlign;
    ExpandType expandTypeWidth{ExpandType::width_fixed};
    ExpandType expandTypeHeight{ExpandType::height_fixed};
    std::weak_ptr<Widget> parent;
    mutable std::optional<WidgetSize> optWidgetSize;
    mutable std::optional<WidgetSize> optWidgetMinSize;
    bool arrangeNecessary{true};
    WidgetId widgetId;
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
