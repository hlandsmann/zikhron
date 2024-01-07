#include <TextToken.h>
#include <TextTokenSeq.h>
#include <detail/Widget.h>
#include <annotation/Token.h>
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

    lines = create<Box>();
    lines->setPadding(0);
    lines->setBorder(border);
    lines->setOrientationVertical();

    scratchBox = createOrphan<Box>();
    scratchBox->setPadding(0);
    scratchBox->setBorder(border);
    scratchBox->setOrientationVertical();
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
    widgetSize.heightType = layout::ExpandType::expand;
    widgetSize.widthType = layout::ExpandType::expand;
    // widgetSize.width = 1.F;
    // widgetSize.height = 1.F;
    return widgetSize;
}

auto TextTokenSeq::linesFit() const -> bool
{
    const auto& rect = Rect();
    if (lines->getWidgetSize().width > rect.width) {
        return false;
    }
    lines->start();
    auto tokenIt = paragraph.begin();
    while (!lines->isLast()) {
        auto& line = lines->next<Box>();
        line.start();
        while (!line.isLast() && tokenIt < paragraph.end()) {
            line.next<TextToken>();
            tokenIt++;
        }
        if (tokenIt >= paragraph.end()) {
            return true;
        }
        scratchBox->clear();
        addTextToken(*scratchBox, *tokenIt);
        if (scratchBox->getWidgetSize().width + line.getWidgetSize().width <= rect.width) {
            return false;
        }
    }

    return true;
}

void TextTokenSeq::addTextToken(Box& box, const annotation::Token& token)
{
    auto textToken = box.add<TextToken>(Align::start, token);
    textToken->setFontType(context::FontType::chineseBig);
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

    const auto& rect = Rect();
    // auto width = lines->getWidgetSize().width;
    // spdlog::critical("x: {}, y: {}, w: {}, h: {}", rect.x, rect.y, rect.width, rect.height);
    imglog::log("ttq, x {}, y {}, w{}, h{}", rect.x, rect.y, rect.width, rect.height);
    spdlog::info("ttq, x {}, y {}, w{}, h{}", rect.x, rect.y, rect.width, rect.height);
    lines->clear();
    auto line = lines->add<Box>(Align::start);
    for (const auto& token : paragraph) {
        // auto textToken = line->add<TextToken>(Align::start, token);
        // textToken->setFontType(context::FontType::chineseBig);
        addTextToken(*line, token);
        if (lines->getWidgetSize().width > rect.width) {
            line->pop();
            line = lines->add<Box>(Align::start);
            addTextToken(*line, token);
        }
        // spdlog::info("{}", token.getValue());
    }
    // spdlog::warn("width: {}", width);
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
