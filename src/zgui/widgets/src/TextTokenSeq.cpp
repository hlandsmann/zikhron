#include <TextTokenSeq.h>
#include <Widget.h>
#include <imgui.h>

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

void TextTokenSeq::arrange()
{
}

void TextTokenSeq::draw()
{
}

} // namespace widget
