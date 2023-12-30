#include <TextToken.h>
#include <TextTokenSeq.h>
#include <Widget.h>
#include <context/Fonts.h>
#include <imgui.h>
#include <spdlog/spdlog.h>

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
    arrange();
}

auto TextTokenSeq::calculateSize() const -> WidgetSize
{
    return lines.getWidgetSize();
}

void TextTokenSeq::arrange()
{
    using Align = widget::layout::Align;
    // auto it = paragraph.begin();

    for (const auto& token : paragraph) {
        auto textToken = lines.add<TextToken>(Align::start, token);
        textToken->setFontType(context::FontType::chineseBig);
        spdlog::info("{}", token.getValue());
    }
    lines.setPadding(0);
}

void TextTokenSeq::draw()
{
    lines.arrange();
    while (!lines.isLast()) {
        auto& textToken = lines.next<TextToken>();
        textToken.clicked();
    }
}

} // namespace widget
