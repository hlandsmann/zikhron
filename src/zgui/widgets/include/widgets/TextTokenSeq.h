#pragma once
#include "Box.h"
#include "TextToken.h"
#include "detail/Widget.h" // IWYU pragma: export detail/Widget.h

#include <annotation/Token.h>
#include <annotation/TokenText.h>
#include <context/Fonts.h>
#include <imgui.h>

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace widget {
class TextTokenSeq : public Widget
{
public:
    struct Config
    {
        context::FontType fontType = context::FontType::Gui;
        float wordPadding = 0.F;
        float linePadding = 0.F;
        float border = 0.F;
    };

private:
    using Paragraph = annotation::TokenText::Paragraph;
    using Align = widget::layout::Align;

    template<class T>
    friend class MetaBox;
    friend class Widget;
    void setup(const Paragraph& paragraph);
    void setup(const Paragraph& paragraph, const Config& config);

public:
    TextTokenSeq(WidgetInit init);

    auto draw() -> std::optional<std::shared_ptr<TextToken>>;

private:
    [[nodiscard]] auto calculateSize() const -> WidgetSize override;
    [[nodiscard]] auto calculateMinSize() const -> WidgetSize override;

    auto arrange(const layout::Rect& rect) -> bool override;
    [[nodiscard]] auto getWidgetSizeFromRect(const layout::Rect& rect) -> WidgetSize override;

    auto arrangeLines(Box& lines, const layout::Rect& rect) -> bool;
    auto linesFit(const layout::Rect& rect) const -> bool;
    void addTextToken(Box& box, const annotation::Token& token) const;
     auto addLine(Box& lines)const -> std::shared_ptr<widget::Box>;
    layout::Rect lastRect;
    std::shared_ptr<Box> lineBox;
    std::shared_ptr<Box> scratchBox;
    Paragraph paragraph;

    Config config;
};

} // namespace widget
