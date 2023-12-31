#include <TextToken.h>
#include <TextTokenSeq.h>
#include <Widget.h>
#include <context/Fonts.h>
#include <context/imglog.h>
#include <imgui.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <memory>
#include <string>
#include <utility>

namespace ranges = std::ranges;
namespace widget {
void TextTokenSeq::setup(Paragraph _paragraph)
{
    paragraph = std::move(_paragraph);
    lines = std::make_shared<Box>(getThemePtr(), PassiveOrientation(), std::weak_ptr{shared_from_this()});
    lines->setPadding(0);
    lines->setBorder(0);
    lines->setOrientationVertical();
}

TextTokenSeq::TextTokenSeq(WidgetInit init)
    : Widget{init}
{
}

auto TextTokenSeq::calculateSize() const -> WidgetSize
{
    auto size = lines->getExpandedSize();
    // spdlog::critical("w: {}, h: {}, we: {}, he: {}", size.width, size.height, size.widthType, size.heightType);
    // return lines->getWidgetSize();
    auto widgetSize = lines->getWidgetSize();
    widgetSize.heightType = layout::SizeType::expand;
    widgetSize.widthType = layout::SizeType::expand;
    widgetSize.width = 1.F;
    widgetSize.height = 1.F;
    return widgetSize;
}

auto TextTokenSeq::linesFit() const -> bool
{
    const auto& rect = Rect();
    bool allLinesFit = true;
    while (!lines->isLast()) {
        auto line = lines->next<Box>();
        allLinesFit &= (line.getWidgetSize().width <= rect.width);
    }
    return allLinesFit;
}

auto TextTokenSeq::arrange() -> bool
{
    lines->start();
    if (paragraph.empty()) {
        return lines->arrange();
    }
    if (!lines->isLast() && linesFit()) {
        return lines->arrange();
    }
    // auto it = paragraph.begin();

    // lines.setOrientationVertical();
    // lines.setFlipChildrensOrientation(false);
    // spdlog::warn("rect w: {}, h: {}", Rect().width, Rect().height);

    using Align = widget::layout::Align;
    const auto& rect = Rect();
    spdlog::critical("w: {}, h: {}", rect.width, rect.height);
    imglog::log("ttq, x {}, y {}, w{}, h{}", rect.x, rect.y, rect.width, rect.height);
    lines->clear();
    auto line = lines->add<Box>(Align::start);
    bool lineAdded = true;
    for (const auto& token : paragraph) {
        auto textToken = line->add<TextToken>(Align::start, token);
        textToken->setFontType(context::FontType::chineseBig);
        if (line->getWidgetSize().width > rect.width) {
            line->pop();
            line = lines->add<Box>(Align::start);
            textToken = line->add<TextToken>(Align::start, token);
            textToken->setFontType(context::FontType::chineseBig);
        }
        // spdlog::info("{}", token.getValue());
    }
    // resetWidgetSize();
    return lines->arrange();
    // return true;
}

void TextTokenSeq::draw()
{
    lines->start();
    while (!lines->isLast()) {
        auto& line = lines->next<Box>();
        line.start();
        while (!line.isLast()) {
            auto& textToken = line.next<TextToken>();
            textToken.clicked();
        }
    }
}

} // namespace widget
