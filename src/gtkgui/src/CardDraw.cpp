#include <CardDraw.h>
#include <DataThread.h>
#include <spdlog/spdlog.h>
#include <boost/range/combine.hpp>
#include <ranges>
#include <algorithm>

namespace ranges = std::ranges;

CardDraw::CardDraw(Gtk::Overlay& overlay_in) : overlay(overlay_in) { set_column_spacing(48); }

void CardDraw::setParagraph(const std::shared_ptr<markup::Paragraph>& paragraph_in) {
    paragraph = paragraph_in;
    int textDrawIndex = 0;  // ToDo: use enumerate here
    for (const auto& fragment : paragraph->getFragments()) {
        addTextDraw(textDrawIndex % 2, textDrawIndex / 2, fragment);
        textDrawIndex++;
    }
    setupSignals();
}

void CardDraw::addTextDraw(int column, int row, const std::string& markup) {
    auto textDraw = std::make_unique<TextDraw>();
    textDraw->setFontSize(textFontSize);
    textDraw->setSpacing(textSpacing);
    textDraw->setText(markup);
    attach(*textDraw, column, row);
    textDrawContainer.push_back(std::move(textDraw));
}

void CardDraw::setupSignals() {
    int textDrawIndex = 0;  // ToDo: use enumerate here
    for (auto& textDraw : textDrawContainer) {
        textDraw->signal_hoverByteIndex([this, textDrawIndex](int byteIndex) {
            if (isAnnotation)
                mouseHoverAnnotation(textDrawIndex, byteIndex);
            else
                mouseHoverStandard(textDrawIndex, byteIndex);
        });
        textDraw->signal_mouseLeave([this, textDrawIndex]() {
            paragraph->undoChange();
            textDrawContainer[textDrawIndex]->update_markup(paragraph->getFragments()[textDrawIndex]);
        });
        textDraw->signal_mouseClickByteIndex([this, textDrawIndex](int byteIndex) {
            if (isAnnotation)
                mouseClickAnnotation(textDrawIndex, byteIndex);
            else
                mouseClickStandard(textDrawIndex, byteIndex);
        });
        textDrawIndex++;
    }
}

void CardDraw::mouseHoverAnnotation(int textDrawIndex, int byteIndex) {
    auto startIndexPos = paragraph->fragmentStartPos(textDrawIndex, paragraph->BytePositions());

    paragraph->undoChange();
    if (byteIndex != -1)
        paragraph->highlightAnnotationAtPosition(startIndexPos + byteIndex);
    textDrawContainer[textDrawIndex]->update_markup(paragraph->getFragments()[textDrawIndex]);
}

void CardDraw::mouseHoverStandard(int textDrawIndex, int byteIndex) {
    auto startIndexPos = paragraph->fragmentStartPos(textDrawIndex, paragraph->BytePositions());

    paragraph->undoChange();
    if (byteIndex != -1)
        paragraph->highlightWordAtPosition(startIndexPos + byteIndex, paragraph->BytePositions());
    textDrawContainer[textDrawIndex]->update_markup(paragraph->getFragments()[textDrawIndex]);
}

void CardDraw::mouseClickAnnotation(int textDrawIndex, int byteIndex) {
    auto startIndexPos = paragraph->fragmentStartPos(textDrawIndex, paragraph->BytePositions());

    auto [activeChoice, marked, unmarked, newPos, combinations, characterSequence] =
        paragraph->getAnnotationPossibilities(byteIndex + startIndexPos);
    if (marked.empty())
        return;

    auto rect = textDrawContainer[textDrawIndex]->getCharacterPosition(int(newPos - startIndexPos));
    auto annotationOverlayInit = AnnotationOverlayInit{
        .activeChoice = activeChoice,
        .marked = marked,
        .unmarked = unmarked,
        .x = rect.get_x(),
        .y = rect.get_y(),
        .x_max = overlay.get_size(Gtk::Orientation::HORIZONTAL),
        .y_max = overlay.get_size(Gtk::Orientation::VERTICAL)};
    annotationOverlay = std::make_unique<AnnotationOverlay>(annotationOverlayInit);
    annotationOverlay->signal_annotationChoice(
        [this, combinations, characterSequence](std::optional<int> choice) {
            overlay.remove_overlay(*annotationOverlay);

            if (not choice.has_value())
                return;
            DataThread::get().submitAnnotation(combinations.at(choice.value()), characterSequence);
        });
    overlay.add_overlay(*annotationOverlay);
}

void CardDraw::mouseClickStandard(int textDrawIndex, int byteIndex) {
    auto startIndexPos = paragraph->fragmentStartPos(textDrawIndex, paragraph->BytePositions());
    const auto clickedItem = paragraph->wordFromPosition(startIndexPos + byteIndex,
                                                         paragraph->BytePositions());
    if (clickedItem.empty())
        return;
    const auto& word = clickedItem.front();
    spdlog::info("{}", word.key);
    for (const auto& word : clickedItem) {
        spdlog::info("    {}", word.pronounciation);
        spdlog::info("        {}", word.meanings.front());
    }

    auto newPos = paragraph->getWordStartPosition(startIndexPos + byteIndex, paragraph->BytePositions());
    auto rect = textDrawContainer[textDrawIndex]->getCharacterPosition(int(newPos - startIndexPos));

    std::vector<std::string> pronounciations;
    std::vector<std::string> meanings;
    ranges::transform(clickedItem, std::back_inserter(pronounciations), &ZH_Dictionary::Item::pronounciation);
}