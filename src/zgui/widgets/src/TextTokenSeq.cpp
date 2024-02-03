#include <Box.h>
#include <TextToken.h>
#include <TextTokenSeq.h>
#include <annotation/Token.h>
#include <context/Fonts.h>
#include <context/imglog.h>
#include <detail/Widget.h>
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

    lines = create<Box>(Orientation::vertical);
    lines->setPadding(0);
    lines->setBorder(border);

    scratchBox = createOrphan<Box>(Orientation::vertical);
    scratchBox->setPadding(0);
    scratchBox->setBorder(border);
}

TextTokenSeq::TextTokenSeq(WidgetInit init)
    : Widget{init}
{
}

auto TextTokenSeq::calculateSize() const -> WidgetSize
{
    // spdlog::critical("w: {}, h: {}, we: {}, he: {}", size.width, size.height, size.widthType, size.heightType);
    // return lines->getWidgetSize();
    auto widgetSize = lines->getWidgetSize();
    // widgetSize.width = 1.F;
    // widgetSize.height = 1.F;
    return widgetSize;
}

auto TextTokenSeq::linesFit() const -> bool
{
    const auto& rect = getRect();
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

auto TextTokenSeq::arrange(const layout::Rect& rect) -> bool
{
    lines->start();
    if (paragraph.empty()) {
        return lines->arrange(rect);
    }
    if (!lines->isLast() && linesFit()) {
        return lines->arrange(rect);
    }

    // auto width = lines->getWidgetSize().width;
    // spdlog::critical("x: {}, y: {}, w: {}, h: {}", rect.x, rect.y, rect.width, rect.height);
    imglog::log("ttq, x {}, y {}, w{}, h{}", rect.x, rect.y, rect.width, rect.height);
    spdlog::info("ttq, x {}, y {}, w{}, h{}", rect.x, rect.y, rect.width, rect.height);
    lines->clear();
    auto line = lines->add<Box>(Align::start, Orientation::horizontal);
    for (const auto& token : paragraph) {
        // auto textToken = line->add<TextToken>(Align::start, token);
        // textToken->setFontType(context::FontType::chineseBig);
        addTextToken(*line, token);
        if (lines->getWidgetSize().width > rect.width) {
            line->pop();
            line = lines->add<Box>(Align::start, Orientation::horizontal);
            addTextToken(*line, token);
        }
        // spdlog::info("{}", token.getValue());
    }
    // spdlog::warn("width: {}", width);
    // resetWidgetSize();
    return lines->arrange(rect);
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
