#include <CardDraw.h>
#include <spdlog/spdlog.h>
#include <boost/range/combine.hpp>

CardDraw::CardDraw() { set_column_spacing(48); }

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
        textDraw->signal_hoverByteIndex([this,
                                         index,
                                         startIndexPos = paragraph->fragmentStartPos(
                                             index, paragraph->BytePositions())](int byteIndex) {
            paragraph->undoChange();
            if (byteIndex != -1)
                paragraph->highlightWordAtPosition(startIndexPos + byteIndex,
                                                   paragraph->BytePositions());
            textDrawContainer[index]->update_markup(paragraph->getFragments()[index]);
        });
        textDraw->signal_mouseLeave([this, index]() {
            paragraph->undoChange();
            textDrawContainer[index]->update_markup(paragraph->getFragments()[index]);
        });
        index++;
    }
}
void CardDraw::setModeAnnotation(bool annotation) {
    paragraph->updateAnnotationColoring();
    auto fragments = paragraph->getFragments();
    for (const auto& [textDraw, fragment] :
         boost::combine(textDrawContainer, fragments)) {
             spdlog::info("Frag: {}", fragment);
        textDraw->setText(fragment);
    }
}
