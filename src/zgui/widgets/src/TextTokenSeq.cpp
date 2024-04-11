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
#include <optional>
#include <string>

namespace ranges = std::ranges;

namespace widget {
void TextTokenSeq::setup(const Paragraph& _paragraph)
{
    using namespace widget::layout;
    paragraph = _paragraph;

    lineBox = create<Box>(Orientation::vertical);
    lineBox->setName("linebox");
    lineBox->setExpandType(width_fixed, height_fixed);
    lineBox->setPadding(config.padding);
    lineBox->setBorder(border);

    scratchBox = createOrphan<Box>(Orientation::vertical);
    scratchBox->setExpandType(width_fixed, height_fixed);
    scratchBox->setPadding(config.padding);
    scratchBox->setBorder(border);
    scratchBox->cutWidgetIdGen();
}

void TextTokenSeq::setup(const Paragraph& _paragraph, const Config& _config)
{
    config = _config;
    setup(_paragraph);
}

TextTokenSeq::TextTokenSeq(WidgetInit init)
    : Widget{init}
{
}

auto TextTokenSeq::calculateSize() const -> WidgetSize
{
    // spdlog::critical("w: {}, h: {}, we: {}, he: {}", size.width, size.height, size.widthType, size.heightType);
    // return lines->getWidgetSize();
    auto widgetSize = lineBox->getWidgetSize();
    // widgetSize.width = 1.F;
    // widgetSize.height = 1.F;
    return widgetSize;
}

auto TextTokenSeq::calculateMinSize() const -> WidgetSize
{
    scratchBox->clear();
    for (const auto& token : paragraph) {
        addTextToken(*scratchBox, token);
    }
    float maxWidth = 0;
    float maxHeight = 0;

    scratchBox->start();
    while (!scratchBox->isLast()) {
        auto& textToken = scratchBox->next<TextToken>();
        auto minSize = textToken.getWidgetMinSize();
        maxWidth = std::max(maxWidth, minSize.width);
        maxHeight = std::max(maxHeight, minSize.height);
    }
    return {.width = maxWidth, .height = maxHeight};
}

auto TextTokenSeq::arrangeLines(Box& lines, const layout::Rect& rect) -> bool
{
    // auto width = lines->getWidgetSize().width;
    // spdlog::critical("x: {}, y: {}, w: {}, h: {}", rect.x, rect.y, rect.width, rect.height);
    // spdlog::critical("ttq, x: {}, y: {}, w: {}, h: {}", rect.x, rect.y, rect.width, rect.height);
    // imglog::log("ttq, x {}, y {}, w{}, h{}", rect.x, rect.y, rect.width, rect.height);
    // spdlog::info("ttq, x {}, y {}, w{}, h{}", rect.x, rect.y, rect.width, rect.height);
    lines.clear();
    auto line = lines.add<Box>(Align::start, Orientation::horizontal);
    for (const auto& token : paragraph) {
        // auto textToken = line->add<TextToken>(Align::start, token);
        // textToken->setFontType(context::FontType::chineseBig);
        addTextToken(*line, token);
        if (lines.getWidgetSize().width > rect.width) {
            line->pop();
            line = lines.add<Box>(Align::start, Orientation::horizontal);
            addTextToken(*line, token);
        }
        // spdlog::info("{}", token.getValue());
    }
    // spdlog::warn("width: {}", width);
    // resetWidgetSize();
    return lines.arrange(rect);
    // return true;
}

auto TextTokenSeq::linesFit(const layout::Rect& rect) const -> bool
{
    if (lineBox->getWidgetSize().width > rect.width) {
        imglog::log("{}: 1, false", getName());
        return false;
    }
    lineBox->start();
    auto tokenIt = paragraph.begin();
    while (!lineBox->isLast()) {
        auto& line = lineBox->next<Box>();
        line.start();
        while (!line.isLast() && tokenIt < paragraph.end()) {
            line.next<TextToken>();
            tokenIt++;
        }
        if (tokenIt == paragraph.end()) {
            // imglog::log("{}: 2, true, rectWidth: {}", getName(), rect.width);
            return true;
        }
        scratchBox->clear();
        addTextToken(*scratchBox, *tokenIt);
        // imglog::log("{}:  combWidth: {}", getName(), scratchBox->getWidgetSize().width + line.getWidgetSize().width);
        if (scratchBox->getWidgetSize().width + line.getWidgetSize().width <= rect.width) {
            // imglog::log("{}: 3, false", getName());
            return false;
        }
    }

    // imglog::log("{}: 4, true", getName());
    return true;
}

void TextTokenSeq::addTextToken(Box& box, const annotation::Token& token) const
{
    auto textToken = box.add<TextToken>(Align::start, token);
    textToken->setFontType(config.fontType);
}

auto TextTokenSeq::arrange(const layout::Rect& rect) -> bool
{
    // imglog::log("ttq,arr, x {}, y {}, w{}, h{}", rect.x, rect.y, rect.width, rect.height);
    // spdlog::critical("ttq,arr, x {}, y {}, w{}, h{}", rect.x, rect.y, rect.width, rect.height);
    lineBox->start();
    if (paragraph.empty()) {
        return lineBox->arrange(rect);
    }
    if (!lineBox->isLast() && linesFit(rect)) {
        return lineBox->arrange(rect);
    }
    return arrangeLines(*lineBox, rect);
}

auto TextTokenSeq::getWidgetSizeFromRect(const layout::Rect& rect) -> WidgetSize
{
    //     imglog::log("gwsfr, x {}, y {}, w{}, h{}", rect.x, rect.y, rect.width, rect.height);
    lineBox->start();
    if (paragraph.empty()) {
        return {};
    }
    if (!lineBox->isLast() && linesFit(rect)) {
        return lineBox->getWidgetSizeFromRect(rect);
    }
    arrangeLines(*scratchBox, rect);
    return scratchBox->getWidgetSize();
}

auto TextTokenSeq::draw() -> std::optional<std::shared_ptr<TextToken>>
{
    std::optional<std::shared_ptr<TextToken>> result;
    lineBox->start();
    while (!lineBox->isLast()) {
        auto& line = lineBox->next<Box>();
        line.start();
        while (!line.isLast()) {
            auto& textToken = line.next<TextToken>();
            if (textToken.clicked()) {
                result.emplace(std::dynamic_pointer_cast<TextToken>(textToken.shared_from_this()));
            }
        }
    }
    return result;
}

} // namespace widget
