#pragma once
#include <imgui.h>

#include <memory>
#include <optional>
#include <utility>

namespace widget {
namespace layout {
struct Rect
{
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

class WidgetBase
{
public:
    WidgetBase(layout::Align align, layout::Orientation orientation, std::shared_ptr<layout::Rect> rect);
    virtual ~WidgetBase() = default;
    WidgetBase(const WidgetBase&) = default;
    WidgetBase(WidgetBase&&) = default;
    auto operator=(const WidgetBase&) -> WidgetBase& = default;
    auto operator=(WidgetBase&&) -> WidgetBase& = default;

    [[nodiscard]] virtual auto getWidgetSize() const -> const WidgetSize& = 0;
    [[nodiscard]] auto Align() const -> layout::Align;
    [[nodiscard]] auto Orientation() const -> layout::Orientation;

protected:
    [[nodiscard]] auto Rect() const -> const layout::Rect&;

private:
    layout::Align baseAlign;
    layout::Orientation baseOrientation;
    std::shared_ptr<layout::Rect> rectPtr;
};

template<class WidgetImpl>
class Widget : public WidgetBase
{
public:
    Widget(layout::Align _align, layout::Orientation _orientation, std::shared_ptr<layout::Rect> _rect)
        : WidgetBase{_align, _orientation, std::move(_rect)} {}
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

private:
    mutable std::optional<WidgetSize> optWidgetSize;
};

} // namespace widget
