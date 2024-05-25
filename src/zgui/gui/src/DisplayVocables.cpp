#include "DisplayVocables.h"

#include <annotation/Ease.h>
#include <annotation/Token.h>
#include <annotation/TokenText.h>
#include <annotation/Word.h>
#include <annotation/WordDB.h>
#include <dictionary/ZH_Dictionary.h>
#include <misc/Identifier.h>
#include <widgets/Grid.h>
#include <widgets/Layer.h>
#include <widgets/TextTokenSeq.h>
#include <widgets/ToggleButtonGroup.h>

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

namespace ranges = std::ranges;
namespace views = ranges::views;

namespace gui {
DisplayVocables::DisplayVocables(std::shared_ptr<widget::Layer> _layer,
                                 std::shared_ptr<annotation::WordDB> _wordDB,
                                 std::vector<ActiveVocable>&& _orderedVocId_ease)
    : layer{std::move(_layer)}
    , wordDB{std::move(_wordDB)}
    , activeVocables{std::move(_orderedVocId_ease)}
    , words{wordsFromActiveVocables(activeVocables, wordDB)}

{
    setup();
}

void DisplayVocables::draw()
{
    layer->start();
    auto& grid = layer->next<widget::Grid>();
    grid.start();
    std::size_t index = 0;
    while (!grid.isLast()) {
        grid.next<widget::TextTokenSeq>().draw(); // key
        grid.next<widget::TextTokenSeq>().draw(); // pronounciation
        grid.next<widget::TextTokenSeq>().draw(); // meaning

        auto& ease = activeVocables[index].ease;
        auto easeVal = grid.next<widget::ToggleButtonGroup>().Active(mapEaseToUint(ease.easeVal));
        ease.easeVal = mapIntToEase(static_cast<unsigned>(easeVal));
        index++;
    }
}

void DisplayVocables::setup()
{
    using namespace widget::layout;
    auto grid = layer->add<widget::Grid>(Align::start, 4, widget::Grid::Priorities{0.1F, 0.2F, 0.4F, 0.3F});
    grid->setBorder(16.F);
    grid->setHorizontalPadding(64.F);
    grid->setVerticalPadding(16.F);
    grid->setExpandType(width_fixed, height_fixed);
    for (auto& [vocId, ease, colorId] : activeVocables) {
        const auto& word = wordDB->lookupId(vocId);
        addVocable(*grid, *word, colorId);
        addEaseButtonGroup(*grid);
    }
}

void DisplayVocables::addVocable(widget::Grid& grid, const Word& word, ColorId colorId)
{
    using annotation::tokenVectorFromString;

    widget::TextTokenSeq::Config ttqConfig;
    ttqConfig.fontType = fontType;
    ttqConfig.wordPadding = 15.F;
    grid.add<widget::TextTokenSeq>(Align::start, tokenVectorFromString(word.Key(), colorId), ttqConfig);
    const auto& option = word.getDefinitions().front();
    grid.add<widget::TextTokenSeq>(Align::start, tokenVectorFromString(option.pronounciation, colorId), ttqConfig);
    grid.add<widget::TextTokenSeq>(Align::start, tokenVectorFromString(option.meanings.at(0), colorId), ttqConfig);
}

void DisplayVocables::addEaseButtonGroup(widget::Grid& grid)
{
    auto tbg = grid.add<widget::ToggleButtonGroup>(Align::start, widget::Orientation::horizontal,
                                                   std::initializer_list<std::string>{"Again", "Hard", "Normal", "Easy"});
    tbg->setVerticalAlign(Align::center);
}

auto DisplayVocables::wordsFromActiveVocables(const std::vector<ActiveVocable>& activeVocables,
                                              std::shared_ptr<annotation::WordDB> wordDB)
        -> std::vector<std::shared_ptr<Word>>
{
    auto words = std::vector<std::shared_ptr<Word>>{};
    // clang-format off
    ranges::transform(activeVocables,
                      std::back_inserter(words),
                      [&wordDB](VocableId vocId) { return wordDB->lookupId(vocId); },
                      &ActiveVocable::vocableId);
    //clang-foramt on

    return words;
}

} // namespace gui
