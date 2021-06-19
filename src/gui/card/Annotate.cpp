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
    auto [marked, unmarked, newPos, _combinations, _characters] = paragraph->getAnnotationPossibilities(
        pos);

    if (marked.empty())
        return;

    combinations = std::move(_combinations);
    characters = std::move(_characters);

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
               fmt::join(characters, ""));
    QList<int> qtCombination;
    QList<QString> qtCharacters;

    ranges::copy(combinations.at(index), std::back_inserter(qtCombination));
    ranges::transform(characters, std::back_inserter(qtCharacters), QString::fromStdString);
    emit cardAnnotationChoice(qtCombination, qtCharacters);
}

void Annotate::getAnnotation(const PtrParagraph &_paragraph) {
    paragraph = _paragraph.get();
    paragraph->updateAnnotationColoring();
    emit textUpdate(QString::fromStdString(paragraph->get()));
}
}  // namespace card
