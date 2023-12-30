#include <TextToken.h>
#include <TextTokenSeq.h>
#include <Widget.h>
#include <context/Fonts.h>
#include <imgui.h>
#include <spdlog/spdlog.h>
#include <context/imglog.h>

#include <algorithm>
#include <string>
#include <utility>

namespace ranges = std::ranges;
namespace widget {
TextTokenSeq::TextTokenSeq(WidgetInit init, Paragraph _paragraph)
    : Widget<TextTokenSeq>{init}
    , lines{std::move(init)}
    , paragraph{std::move(_paragraph)}
{
}

auto TextTokenSeq::calculateSize() const -> WidgetSize
{
    return lines.getWidgetSize();
}

auto TextTokenSeq::arrange() -> bool
{
    using Align = widget::layout::Align;
    // auto it = paragraph.begin();

    lines.setPadding(0);
    lines.setBorder(16);
    lines.setOrientationHorizontal();
    // lines.setOrientationVertical();
    // lines.setFlipChildrensOrientation(false);
    // spdlog::warn("rect w: {}, h: {}", Rect().width, Rect().height);
    const auto& rect = Rect();
    imglog::log("ttq, x {}, y {}, w{}, h{}", rect.x, rect.y, rect.width, rect.height);
    lines.clear();
    for (const auto& token : paragraph) {
        auto textToken = lines.add<TextToken>(Align::start, token);
        textToken->setFontType(context::FontType::chineseBig);
        // spdlog::info("{}", token.getValue());
    }
    return lines.arrange();
    // return true;
}

void TextTokenSeq::draw()
{
    lines.start();
    while (!lines.isLast()) {
        auto& textToken = lines.next<TextToken>();
        textToken.clicked();
    }
}

} // namespace widget
