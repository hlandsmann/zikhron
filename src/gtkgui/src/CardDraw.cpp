#include <CardDraw.h>
#include <DataThread.h>
#include <spdlog/spdlog.h>
#include <boost/range/combine.hpp>

CardDraw::CardDraw(Gtk::Overlay& overlay_in) : overlay(overlay_in) { set_column_spacing(48); }

void CardDraw::setParagraph(const std::shared_ptr<markup::Paragraph>& paragraph_in) {
    paragraph = paragraph_in;
    int index = 0;  // ToDo: use enumerate here
    for (const auto& fragment : paragraph->getFragments()) {
        addTextDraw(index % 2, index / 2, fragment);
        index++;
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
    int index = 0;  // ToDo: use enumerate here
    for (auto& textDraw : textDrawContainer) {
        auto startIndexPos = paragraph->fragmentStartPos(index, paragraph->BytePositions());
        textDraw->signal_hoverByteIndex([this, index, startIndexPos](int byteIndex) {
            if (isAnnotation)
                mouseHoverAnnotation(index, startIndexPos, byteIndex);
            else
                mouseHoverStandard(index, startIndexPos, byteIndex);
        });
        textDraw->signal_mouseLeave([this, index]() {
            paragraph->undoChange();
            textDrawContainer[index]->update_markup(paragraph->getFragments()[index]);
        });
        textDraw->signal_mouseClickByteIndex([this, index](int byteIndex) {
            if (isAnnotation)
                mouseClickAnnotation(index, byteIndex);
            else
                mouseClickStandard(index, byteIndex);
        });
        index++;
    }
}

void CardDraw::mouseHoverAnnotation(int index, int startIndexPos, int byteIndex) {
    paragraph->undoChange();
    if (byteIndex != -1)
        paragraph->highlightAnnotationAtPosition(startIndexPos + byteIndex);
    textDrawContainer[index]->update_markup(paragraph->getFragments()[index]);
}

void CardDraw::mouseHoverStandard(int index, int startIndexPos, int byteIndex) {
    paragraph->undoChange();
    if (byteIndex != -1)
        paragraph->highlightWordAtPosition(startIndexPos + byteIndex, paragraph->BytePositions());
    textDrawContainer[index]->update_markup(paragraph->getFragments()[index]);
}

void CardDraw::mouseClickAnnotation(int index, int byteIndex) {
    auto [activeChoice, marked, unmarked, newPos, combinations, characterSequence] =
        paragraph->getAnnotationPossibilities(byteIndex);
    if (marked.empty())
        return;

    auto rect = textDrawContainer[index]->getCharacterPosition(int(newPos));
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

void CardDraw::mouseClickStandard(int index, int byteIndex) {}
