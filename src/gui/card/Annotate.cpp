#include "Annotate.h"
#include <fmt/format.h>
#include <algorithm>
#include <ranges>

namespace ranges = std::ranges;

namespace card {
void Annotate::hoveredTextPosition(int pos) {
    static int lastPos = -1;
    if (lastPos == pos)
        return;
    lastPos = pos;
    paragraph->undoChange();
    paragraph->highlightAnnotationAtPosition(pos);
    emit textUpdate(QString::fromStdString(paragraph->get()));
}

void Annotate::clickedTextPosition(int pos) {
    auto [marked, unmarked, newPos, _combinations, _characterSequence] = paragraph->getAnnotationPossibilities(
        pos);

    if (marked.empty())
        return;

    combinations = std::move(_combinations);
    characterSequence = std::move(_characterSequence);

    QList<QString> qtMarked;
    QList<QString> qtUnmarked;
    ranges::transform(marked, std::back_inserter(qtMarked), QString::fromStdString);
    ranges::transform(unmarked, std::back_inserter(qtUnmarked), QString::fromStdString);
    emit annotationPossibilities(qtMarked, qtUnmarked, newPos);
}

void Annotate::chosenAnnotation(int index) {
    assert(index >= 0);
    assert(static_cast<size_t>(index) < combinations.size());

    fmt::print("Chosen combination [{}] of {}\n",
               fmt::join(combinations.at(index), ","),
               fmt::join(characterSequence, ""));
    QList<int> qtCombination;
    QList<QString> qtCharacterSequence;

    ranges::copy(combinations.at(index), std::back_inserter(qtCombination));
    ranges::transform(characterSequence, std::back_inserter(qtCharacterSequence), QString::fromStdString);
    emit cardAnnotationChoice(qtCombination, qtCharacterSequence);
}

void Annotate::getAnnotation(const PtrParagraph &_paragraph) {
    paragraph = _paragraph.get();
    paragraph->updateAnnotationColoring();
    emit textUpdate(QString::fromStdString(paragraph->get()));
}
}  // namespace card
