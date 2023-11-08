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
    expand
};
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
    WidgetBase(layout::Align align, std::shared_ptr<layout::Rect> rect);
    virtual ~WidgetBase() = default;
    WidgetBase(const WidgetBase&) = default;
    WidgetBase(WidgetBase&&) = default;
    auto operator=(const WidgetBase&) -> WidgetBase& = default;
    auto operator=(WidgetBase&&) -> WidgetBase& = default;

    [[nodiscard]] virtual auto getWidgetSize() const -> const WidgetSize& = 0;
    [[nodiscard]] auto Align() const -> layout::Align;

protected:
    [[nodiscard]] auto Rect() const -> const layout::Rect&;

private:
    layout::Align align;
    std::shared_ptr<layout::Rect> rect;
};

template<class WidgetImpl>
class Widget : public WidgetBase
{
public:
    Widget(layout::Align align, std::shared_ptr<layout::Rect> rect)
        : WidgetBase{align, std::move(rect)} {}
    ~Widget() override = default;
    Widget(const Widget&) = default;
    Widget(Widget&&) = default;
    auto operator=(const Widget&) -> Widget& = default;
    auto operator=(Widget&&) -> Widget& = default;

    [[nodiscard]] auto getWidgetSize() const -> const WidgetSize& override
    {
        if (widgetSize.has_value()) {
            return *widgetSize;
        }
        return widgetSize.emplace(static_cast<WidgetImpl const*>(this)->calculateSize());
    }

private:
    mutable std::optional<WidgetSize> widgetSize;
};

} // namespace widget
