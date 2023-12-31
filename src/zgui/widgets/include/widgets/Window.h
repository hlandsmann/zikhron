#pragma once
#include "Box.h"
#include "Widget.h"

#include <context/Drop.h>
#include <context/Theme.h>
#include <imgui.h>

#include <memory>
#include <string>

namespace widget {

class WindowDrop;

class Window : public Widget<Window>
{
    friend class Box;
    void setup(layout::SizeType sizeTypeWidth,
               layout::SizeType sizeTypeHeight,
               std::string name);

public:
    Window(const WidgetInit& init);
    ~Window() override = default;

    Window(const Window&) = default;
    Window(Window&&) = default;
    auto operator=(const Window&) -> Window& = default;
    auto operator=(Window&&) -> Window& = default;

    [[nodiscard]] auto dropWindow() -> WindowDrop;
    [[nodiscard]] auto arrange() -> bool override;
    [[nodiscard]] auto getBox() -> Box&;

private:
    friend class Widget<Window>;
    auto calculateSize() const -> WidgetSize;
    std::shared_ptr<Box> box;
    layout::SizeType sizeTypeWidth{};
    layout::SizeType sizeTypeHeight{};

    std::string name;
};

class WindowDrop : public context::Drop<WindowDrop>
{
public:
    WindowDrop(const std::string& name,
               const widget::layout::Rect& rect,
               context::StyleColorsDrop styleColorsDrop);

private:
    friend class Drop<WindowDrop>;
    static void pop();
    context::StyleColorsDrop styleColorsDrop;
};
} // namespace widget
