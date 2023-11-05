#include "Layout.h"
#include <string>
#include "Widget.h"

#include <imgui.h>

#include <memory>

namespace widget {

class Window : public Widget<Window>
{
public:
    Window(std::shared_ptr<layout::rect> rect, float width, float height, std::string name);
    ~Window() override = default;
    Window(const Window&) = default;
    Window(Window&&) = default;
    auto operator=(const Window&) -> Window& = default;
    auto operator=(Window&&) -> Window& = default;

private:
    friend class Widget<Window>;
    auto calculateSize() const -> WidgetSize;
    std::shared_ptr<layout::rect> rect;
    float width;
    float height;
    std::string name;
};
} // namespace widget
