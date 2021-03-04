#include "Annotate.h"

namespace card {
Annotate::Annotate() { setFiltersChildMouseEvents(true); }

void Annotate::getDictionary(const PtrDictionary &_zh_dict) {
    zh_dict = _zh_dict.get();
    useCard();
}
void Annotate::getCard(const PtrCard &_ptrCard) {
    ptrCard = _ptrCard.get();
    useCard();
}

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
}  // namespace card
