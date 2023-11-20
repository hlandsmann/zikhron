#include <Button.h>
#include <Widget.h>
#include <context/Theme.h>
#include <context/imglog.h>
#include <imgui.h>

#include <memory>
#include <string>
#include <utility>
namespace widget {

Button::Button(std::shared_ptr<context::Theme> _theme,
               layout::Orientation _orientation,
               layout::Align _align,
               std::shared_ptr<layout::Rect> _rect,
               std::string _label)
    : Widget<Button>{std::move(_theme), _orientation, _align, std::move(_rect)}
    , label{std::move(_label)}
{}

auto Button::calculateSize() const -> WidgetSize
{
    // ImDrawList* list = ImGui::GetWindowDrawList();

    // list->ChannelsSplit(2);

    // auto id = label + "{invisible}";
    // ImGui::PushID(id.c_str());
    // list->ChannelsSetCurrent(1);
    ImGui::Button(label.c_str());
    // ImGui::PopID();

    auto size = ImGui::GetItemRectSize();

    // list->ChannelsSetCurrent(0);

    return {.widthType = Orientation() == layout::Orientation::horizontal ? layout::width_expand : layout::width_fixed,
            .heightType = Orientation() == layout::Orientation::vertical ? layout::height_expand : layout::height_fixed,
            .width = size.x,
            .height = size.y};
}

auto Button::clicked() const -> bool
{
    const auto& btnRect = Rect();
    ImGui::SetCursorPos({btnRect.x, btnRect.y});
    // imglog::log("widgth: {}, o: {}", rect.width, static_cast<int>(Orientation()));
    return ImGui::Button(label.c_str(), {btnRect.width, btnRect.height});
}

// {
//
//     ImGui::Begin("Hello, world!");
//
//     ImDrawList* list =  ImGui::GetWindowDrawList();
//
//     struct foo {
//             bool btn() { return ImGui::Button("btn",ImVec2{200,20}); };
//     } btn;
//
//     ImVec2 start = ImGui::GetCursorPos();
//     ImVec2 size;
//
//     list->ChannelsSplit(2);
//
//     ImGui::PushID("someotherid");
//     list->ChannelsSetCurrent(1);
//     if (btn.btn() ) {
//             std::cout << "btn 1 (invisible) pressed\n";
//     };
//     ImGui::PopID();
//
//     size = ImGui::GetItemRectSize();
//     ImVec2 available = ImGui::GetContentRegionMax();
//     start.x = (available.x-size.x)/2;
//     ImGui::SetCursorPos(start);
//
//     list->ChannelsSetCurrent(0);
//     if (btn.btn()){
//             std::cout << "btn 1 pressed\n";
//     };
//
//     ImGui::End();
// }
} // namespace widget
