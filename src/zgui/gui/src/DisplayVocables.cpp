#include "DisplayVocables.h"

#include <annotation/Ease.h>
#include <annotation/Token.h>
#include <annotation/TokenText.h>
#include <context/Fonts.h>
#include <database/Word.h>
#include <database/WordDB.h>
#include <dictionary/DictionaryChi.h>
#include <misc/Identifier.h>
#include <misc/Language.h>
#include <utils/format.h>
#include <widgets/Grid.h>
#include <widgets/Layer.h>
#include <widgets/Separator.h>
#include <widgets/TextTokenSeq.h>
#include <widgets/ToggleButtonGroup.h>

#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace ranges = std::ranges;

namespace gui {
DisplayVocables::DisplayVocables(std::shared_ptr<widget::Layer> _layer,
                                 std::shared_ptr<database::WordDB> _wordDB,
                                 std::vector<ActiveVocable>&& _orderedVocId_ease,
                                 Language language)
    : layer{std::move(_layer)}
    , wordDB{std::move(_wordDB)}
    , activeVocables{std::move(_orderedVocId_ease)}
    , fontType{context::getFontType(context::FontSize::small, language)}
{
    setup();
}

void DisplayVocables::setup()
{
    using namespace widget::layout;
    auto grid = layer->add<widget::Grid>(Align::start, gridCfg, 5, widget::Grid::Priorities{0.1F, 0.2F, 0.4F, 0.3F, 0.1F});
    setupVocables(*grid);
}

void DisplayVocables::draw()
{
    layer->start();
    auto& grid = layer->next<widget::Grid>();
    drawVocables(grid);
}

void DisplayVocables::reload()
{
    layer->start();
    auto& grid = layer->next<widget::Grid>();
    setupVocables(grid);
}

auto DisplayVocables::getVocIdEase() const -> VocableId_Ease
{
    VocableId_Ease vocIdEase;
    ranges::transform(activeVocables, std::inserter(vocIdEase, vocIdEase.begin()),
                      [](const auto& idEaseColor) -> std::pair<VocableId, Ease> {
                          const auto& [vocId, ease, colorId] = idEaseColor;
                          return {vocId, ease};
                      });

    return vocIdEase;
}

void DisplayVocables::setupVocables(widget::Grid& grid)
{
    using annotation::tokenVectorFromString;
    widget::TextTokenSeq::Config ttqConfig;
    ttqConfig.fontType = fontType;
    ttqConfig.wordPadding = 15.F;

    grid.clear();
    for (auto& [vocId, ease, colorId] : activeVocables) {
        const auto& word = wordDB->lookupId(vocId);
        bool renderKey = true;
        bool renderEase = true;
        for (const auto& def : word->getDefinitions()) {
            bool renderPronounciation = true;
            for (const auto& meaning : def.meanings) {
                if (renderKey) {
                    grid.add<widget::TextTokenSeq>(Align::start, tokenVectorFromString(word->Key(), colorId), ttqConfig);
                    renderKey = false;
                } else {
                    grid.add<widget::Separator>(Align::start, 0.F, 0.F);
                }
                if (renderPronounciation) {
                    grid.add<widget::TextTokenSeq>(Align::start, tokenVectorFromString(def.pronounciation, colorId), ttqConfig);
                    renderPronounciation = false;
                } else {
                    grid.add<widget::Separator>(Align::start, 0.F, 0.F);
                }
                grid.add<widget::TextTokenSeq>(Align::start, tokenVectorFromString(meaning, colorId), ttqConfig);
                if (renderEase) {
                    addEaseButtonGroup(grid);
                    grid.add<widget::TextTokenSeq>(Align::start, makeEaseLabel(ease, colorId), ttqConfig);
                    renderEase = false;
                } else {
                    grid.add<widget::Separator>(Align::start, 0.F, 0.F);
                    grid.add<widget::Separator>(Align::start, 0.F, 0.F);
                }
            }
        }
    }
}

void DisplayVocables::drawVocables(widget::Grid& grid)
{
    using annotation::tokenVectorFromString;

    grid.start();
    for (auto& [vocId, ease, colorId] : activeVocables) {
        const auto& word = wordDB->lookupId(vocId);
        bool renderKey = true;
        bool renderEase = true;
        for (const auto& def : word->getDefinitions()) {
            bool renderPronounciation = true;
            for (const auto& _ : def.meanings) {
                if (renderKey) {
                    grid.next<widget::TextTokenSeq>().draw();
                    renderKey = false;
                } else {
                    grid.next<widget::Separator>();
                }
                if (renderPronounciation) {
                    grid.next<widget::TextTokenSeq>().draw();
                    renderPronounciation = false;
                } else {
                    grid.next<widget::Separator>();
                }
                grid.next<widget::TextTokenSeq>().draw();
                if (renderEase) {
                    auto easeVal = grid.next<widget::ToggleButtonGroup>().Active(mapEaseToUint(ease.easeVal));
                    auto tmp = std::exchange(ease.easeVal, mapIntToEase(static_cast<unsigned>(easeVal)));

                    auto& ttq = grid.next<widget::TextTokenSeq>();
                    ttq.draw();
                    if (tmp != ease.easeVal) {
                        ttq.setParagraph(makeEaseLabel(ease, colorId));
                    }

                    renderEase = false;
                } else {
                    grid.next<widget::Separator>();
                    grid.next<widget::Separator>();
                }
            }
        }
    }
}

void DisplayVocables::addEaseButtonGroup(widget::Grid& grid)
{
    auto tbg = grid.add<widget::ToggleButtonGroup>(Align::start, widget::Orientation::horizontal,
                                                   std::initializer_list<std::string>{"Again", "Hard", "Normal", "Easy"});
}

auto DisplayVocables::makeEaseLabel(const Ease& ease, ColorId colorId) -> std::vector<annotation::Token>
{
    auto progress = ease.getProgress();
    auto easeLabel = fmt::format("{:.1f}, ({:.1f})", progress.intervalDay, progress.easeFactor);
    return annotation::tokenVectorFromString(easeLabel, colorId);
}

} // namespace gui
