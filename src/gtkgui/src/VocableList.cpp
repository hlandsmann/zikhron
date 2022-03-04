#include <VocableList.h>
#include <algorithm>
#include <boost/range/combine.hpp>
#include <ranges>
namespace ranges = std::ranges;

VocableList::VocableList() { set_column_spacing(32); }

void VocableList::setParagraph(const std::shared_ptr<markup::Paragraph>& paragraph_in,
                               const std::vector<Ease>& easeList) {
    paragraph = paragraph_in;
    int index = 0;
    for (const auto& [vocable, pronounciation, meaning] : paragraph->getVocables()) {
        addTextDraw(0, index, vocable);
        addTextDraw(1, index, pronounciation);
        addTextDraw(2, index, meaning);
        addEaseChoice(3, index);
        index++;
    }

    for (const auto& [easeChoice, ease] : boost::combine(easeChoiceContainer, easeList))
        easeChoice->setEase(ease);
}

void VocableList::addTextDraw(int column, int row, const std::string& markup) {
    auto textDraw = std::make_unique<TextDraw>();
    textDraw->setFontSize(vocableFontSize);
    textDraw->setSpacing(vocableSpacing);
    textDraw->setText(markup);
    attach(*textDraw, column, row);
    textDrawContainer.push_back(std::move(textDraw));
}

void VocableList::addEaseChoice(int column, int row) {
    auto easeChoice = std::make_unique<EaseChoice>();
    attach(*easeChoice, column, row);
    easeChoiceContainer.push_back(std::move(easeChoice));
}

auto VocableList::getChoiceOfEase() const -> std::vector<Ease> {
    std::vector<Ease> easeList;
    ranges::transform(easeChoiceContainer,
                      std::back_inserter(easeList),
                      [](const auto& easeChoice) -> Ease { return easeChoice->getEase(); });
    return easeList;
}
