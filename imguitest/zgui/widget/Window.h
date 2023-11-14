#include "Box.h"
#include "Widget.h"

#include <imgui.h>

#include <memory>
#include <string>

namespace widget {

class WindowDrop;

class Window : public Widget<Window>
{
public:
    Window(layout::Align align, layout::Orientation, const std::shared_ptr<layout::Rect>& rect,
           layout::SizeType sizeTypeWidth, layout::SizeType sizeTypeHeight,
           std::string name);
    ~Window() override = default;

    Window(const Window&) = default;
    Window(Window&&) = default;
    auto operator=(const Window&) -> Window& = default;
    auto operator=(Window&&) -> Window& = default;

    auto dropWindow() -> WindowDrop;
    auto getLayout() -> Box&;

private:
    friend class Widget<Window>;
    auto calculateSize() const -> WidgetSize;
    Box layout;
    // float width;
    // float height;
    layout::SizeType sizeTypeWidth;
    layout::SizeType sizeTypeHeight;

    std::string name;
};

class WindowDrop
{
public:
    WindowDrop(const std::string& name, const widget::layout::Rect& rect);
    ~WindowDrop();

    WindowDrop(const WindowDrop&) = default;
    WindowDrop(WindowDrop&&) = default;
    auto operator=(const WindowDrop&) -> WindowDrop& = default;
    auto operator=(WindowDrop&&) -> WindowDrop& = default;
};
} // namespace widget
