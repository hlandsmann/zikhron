#include "Annotate.h"
#include <fmt/format.h>
#include <algorithm>
#include <ranges>

namespace ranges = std::ranges;

namespace card {
Annotate::Annotate() { setFiltersChildMouseEvents(true); }

auto Annotate::childMouseEventFilter(QQuickItem *, QEvent * /*event*/) -> bool { return false; }

void Annotate::useCard() {
    // if (ptrCard == nullptr || zh_dict == nullptr)
    //     return;
    // auto maxText = ptrCard->getTextVector().front();

    // zh_annotator = std::make_unique<ZH_Annotator>(maxText, zh_dict);
    // std::transform(zh_annotator->Items().begin(),
    //                zh_annotator->Items().end(),
    //                std::back_inserter(paragraph),
    //                [](const ZH_Annotator::Item &item) -> markup::Word {
    //                    if (not item.dicItemVec.empty())
    //                        return {.word = item.text, .color = 0, .backGroundColor = 0x010101};
    //                    return item.text;
    //                });

    // emit textUpdate(QString::fromStdString(paragraph.get()));
}
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
}

void Annotate::getAnnotation(const PtrParagraph &_paragraph) {
    paragraph = _paragraph.get();
    paragraph->updateAnnotationColoring();
    emit textUpdate(QString::fromStdString(paragraph->get()));
    // std::cout << "Pargarph got:  \n" << paragraph->get() << "\n";
    // emit textUpdate(QString::fromStdString(paragraph->get()));
    // // QList<int> vocPosList = {0, 15};
    // QList<int> vocPosList;
    // ranges::copy(paragraph->getVocablePositions(), std::back_inserter(vocPosList));

    // emit vocableUpdate(QString::fromStdString(paragraph->getVocableString()), vocPosList, ease);
}
}  // namespace card
