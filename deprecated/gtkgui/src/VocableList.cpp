#include <ButtonGroup.h>
#include <TextDraw.h>
#include <VocableList.h>
#include <annotation/Ease.h>
#include <markup/Markup.h>
#include <fmt/format.h>

#include <boost/range/combine.hpp>
#include <cstddef>
#include <format>
#include <memory>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

#include <sys/types.h>
namespace ranges = std::ranges;
namespace views = ranges::views;

VocableList::VocableList()
{
    set_column_spacing(32);
}

void VocableList::setParagraph(const std::shared_ptr<markup::Paragraph>& paragraph_in,
                               const std::vector<Ease>& _easeList)
{
    easeList = _easeList;

    reset();

    paragraph = paragraph_in;
    int index = 0;
    for (const auto& [vocable, pronounciation, meaning] : paragraph->getVocables()) {
        addTextDraw(0, index, vocable);
        addTextDraw(1, index, pronounciation);
        addTextDraw(2, index, meaning);
        addEaseChoice(3, index);
        addLabel(4, index);
        index++;
    }

    index = 0;
    for (const auto& [easeChoice, ease, label] : views::zip(easeChoiceContainer, easeList, labelContainer)) {
        easeChoice->observe_active([this, ease, index](uint active) {
            auto tmpEase = ease;
            tmpEase.easeVal = mapIntToEase(active);
            auto progress = tmpEase.getProgress();
            labelContainer[index]->set_label(
                    std::format("{:.1f}, ({:.1f})", progress.intervalDay, progress.easeFactor));
        });
        easeChoice->setActive(mapEaseToUint(ease.easeVal));
        index++;
    }
}

void VocableList::addTextDraw(int column, int row, const std::string& markup)
{
    auto textDraw = std::make_unique<TextDraw>();
    textDraw->setFontSize(vocableFontSize);
    textDraw->setSpacing(vocableSpacing);
    textDraw->setText(markup);
    attach(*textDraw, column, row);
    textDrawContainer.push_back(std::move(textDraw));
}

void VocableList::addEaseChoice(int column, int row)
{
    auto easeChoice = std::make_unique<ButtonGroup>("Again", "Hard", "Normal", "Easy");
    attach(*easeChoice, column, row);
    easeChoiceContainer.push_back(std::move(easeChoice));
}

void VocableList::addLabel(int column, int row)
{
    auto progressLabel = std::make_unique<Gtk::Label>("");
    progressLabel->set_margin_end(16);
    attach(*progressLabel, column, row);
    labelContainer.push_back(std::move(progressLabel));
}

auto VocableList::getChoiceOfEase() -> std::vector<Ease>
{
    for (size_t i = 0; i < easeList.size(); i++) {
        easeList[i].easeVal = mapIntToEase(easeChoiceContainer[i]->getActive());
    }
    return easeList;
}

void VocableList::reset()
{
    for (const auto& textDraw : textDrawContainer) {
        remove(*textDraw);
    }
    for (const auto& easeChoice : easeChoiceContainer) {
        remove(*easeChoice);
    }
    for (const auto& label : labelContainer) {
        remove(*label);
    }
    textDrawContainer.clear();
    easeChoiceContainer.clear();
    labelContainer.clear();
}
