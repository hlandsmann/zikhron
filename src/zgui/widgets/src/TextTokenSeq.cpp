#include <Box.h>
#include <TextToken.h>
#include <TextTokenSeq.h>
#include <annotation/Token.h>
#include <context/Fonts.h>
#include <context/imglog.h>
#include <detail/Widget.h>
#include <imgui.h>
#include <utils/spdlog.h>

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
    setName("ttq");

    lineBox = create<Box>(Orientation::vertical);
    lineBox->setName("linebox");
    lineBox->setExpandType(width_fixed, height_fixed);
    lineBox->setPadding(config.linePadding);
    lineBox->setBorder(config.border);

    scratchBox = createOrphan<Box>(Orientation::vertical);
    scratchBox->setName("scratchBox");
    scratchBox->setExpandType(width_fixed, height_fixed);
    scratchBox->setPadding(config.linePadding);
    scratchBox->setBorder(config.border);
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
    auto widgetSize = lineBox->getWidgetSize();
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
    // imglog::log("ttq minsize:, {} : {}", maxWidth, maxHeight);
    return {.width = maxWidth, .height = maxHeight};
}

auto TextTokenSeq::arrangeLines(Box& lines, const layout::Rect& rect) -> bool
{
    lines.clear();
    // std::string testName = "scratchBox";
    // if (lines.getName() == testName && anyParentHasId(91)) {
    //     // lines.scratchDbg();
    //     parentlog("definitionBox", "arrangeLines, {}: x: {}, w: {}, h: {}, minSize: {}", static_cast<int>(getWidgetId()),
    //               rect.x, rect.width, rect.height, lines.getWidgetMinSize().width);
    // }
    auto line = addLine(lines);
    for (const auto& token : paragraph) {
        addTextToken(*line, token);
        const auto& minSize = lines.getWidgetMinSize();
        // if (lines.getName() == testName) {
        //     parentlog("definitionBox", "arrangeLines-minsize, {}: lmw: {}, lw: {}, t: {}", static_cast<int>(getWidgetId()),
        //               minSize.width, lines.getWidgetSize().width, token.string());
        // }
        if (minSize.width > rect.width) {
            // if (lines.getName() == testName) {
            //     parentlog("definitionBox", "arrangeLines-minsize-fail, {}: lineswidth: {}, rect.width{}", static_cast<int>(getWidgetId()),
            //               minSize.width, rect.width);
            // }
            line->pop();
            line = addLine(lines);
            addTextToken(*line, token);
        }
    }
    return lines.arrange(rect);
}

auto TextTokenSeq::linesFit(const layout::Rect& rect) const -> bool
{
    if (lineBox->getWidgetMinSize().width > rect.width) {
        // parentlog("definitionBox", "ttq0 false: {}, w{} : rw{}", static_cast<int>(getWidgetId()),
        //           lineBox->getWidgetMinSize().width, rect.width);
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
            // parentlog("definitionBox", "ttq1 true: {}, w{} : rw{}, h {} : rh {}", static_cast<int>(getWidgetId()),
            //           lineBox->getWidgetMinSize().width, rect.width,
            //           lineBox->getWidgetMinSize().height, rect.height);
            return true;
        }
        scratchBox->clear();
        addTextToken(*scratchBox, *tokenIt);
        if (scratchBox->getWidgetMinSize().width + line.getWidgetMinSize().width <= rect.width) {
            // parentlog("definitionBox", "ttq2 false: {}, w{} : rw{}", static_cast<int>(getWidgetId()),
            //           scratchBox->getWidgetMinSize().width + lineBox->getWidgetMinSize().width, rect.width);
            return false;
        }
    }

    return true;
}

void TextTokenSeq::addTextToken(Box& box, const annotation::Token& token) const
{
    auto textToken = box.add<TextToken>(Align::start, token);
    textToken->setFontType(config.fontType);
}

auto TextTokenSeq::addLine(Box& lines) const -> std::shared_ptr<widget::Box>
{
    using namespace widget::layout;

    auto line = lines.add<Box>(Align::start, Orientation::horizontal);
    line->setExpandType(width_fixed, height_fixed);
    line->setPadding(config.wordPadding);
    return line;
}

auto TextTokenSeq::arrange(const layout::Rect& rect) -> bool
{
    resetWidgetSize();
    lineBox->setName(fmt::format("linebox_{}", getName()));
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
    // imglog::log("ttq, {}: w:{}, h: {}", getName(), rect.width, rect.height);
    lineBox->start();
    if (paragraph.empty()) {
        return {};
    }
    if (!lineBox->isLast() && linesFit(rect)) {
        auto lbs = lineBox->getWidgetSizeFromRect(rect);
        return lbs;
    }
    arrangeLines(*scratchBox, rect);
    auto scratch = scratchBox->getWidgetMinSize();
    return scratch;
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

void TextTokenSeq::setParagraph(const Paragraph& _paragraph)
{
    paragraph = _paragraph;
    lineBox->clear();
    scratchBox->clear();
}

} // namespace widget
