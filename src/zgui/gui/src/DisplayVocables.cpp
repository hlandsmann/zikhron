#include "DisplayVocables.h"
#include <annotation/Word.h>
#include <annotation/WordDB.h>

#include <annotation/Ease.h>
#include <annotation/Token.h>
#include <annotation/TokenText.h>
#include <dictionary/ZH_Dictionary.h>
#include <misc/Identifier.h>
#include <widgets/Grid.h>
#include <widgets/Layer.h>
#include <widgets/TextTokenSeq.h>
#include <widgets/ToggleButtonGroup.h>

#include <cstddef>
#include <initializer_list>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace gui {
DisplayVocables::DisplayVocables(std::shared_ptr<widget::Layer> _layer,
                                 std::shared_ptr<annotation::WordDB> _wordDB,
                                 std::vector<ActiveVocable>&& _orderedVocId_ease)
    : layer{std::move(_layer)}
    , wordDB{std::move(_wordDB)}
    , activeVocables{std::move(_orderedVocId_ease)}

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
    auto grid = layer->add<widget::Grid>(Align::start, 4, widget::Grid::Priorities{0.1F, 0.2F, 0.4F, 0.3F});
    for (auto& [vocId, ease, colorId] : activeVocables) {
        const auto& word = wordDB->lookupId(vocId);
        addVocable(*grid, *word, colorId);
        addEaseButtonGroup(*grid);
    }
}

void DisplayVocables::addVocable(widget::Grid& grid, const annotation::Word& word, ColorId colorId)
{
    widget::TextTokenSeq::Config ttqConfig;
    ttqConfig.fontType = fontType;
    ttqConfig.padding = 15.F;
    grid.add<widget::TextTokenSeq>(Align::start, annotation::tokenVectorFromString(word.Key(), colorId), ttqConfig);
    grid.add<widget::TextTokenSeq>(Align::start, annotation::tokenVectorFromString(word.getPronounciation(), colorId), ttqConfig);
    grid.add<widget::TextTokenSeq>(Align::start, annotation::tokenVectorFromString(word.getMeanings().at(0), colorId), ttqConfig);
}

void DisplayVocables::addEaseButtonGroup(widget::Grid& grid)
{
    auto tbg = grid.add<widget::ToggleButtonGroup>(Align::start, widget::Orientation::horizontal,
                                      std::initializer_list<std::string>{"Again", "Hard", "Normal", "Easy"});
    tbg->setVerticalAlign(Align::center);
}
} // namespace gui
