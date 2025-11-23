#include "DisplayVocables_chi.h"

#include "DisplayVocables.h"

#include <annotation/Ease.h>
#include <annotation/Token.h>
#include <annotation/TokenText.h>
#include <context/Fonts.h>
#include <database/Word.h>
#include <database/WordDB.h>
#include <database/WordDB_chi.h>
#include <dictionary/DictionaryChi.h>
#include <misc/Identifier.h>
#include <misc/Language.h>
#include <spaced_repetition/DataBase.h>
#include <spaced_repetition/Scheduler.h>
#include <utils/format.h>
#include <widgets/Grid.h>
#include <widgets/ImageButton.h>
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
DisplayVocables_chi::DisplayVocables_chi(std::shared_ptr<widget::Layer> _layer,
                                         std::shared_ptr<sr::DataBase> _dataBase,
                                         std::vector<ColoredVocable>&& _coloredVocables,
                                         Language language)
    : layer{std::move(_layer)}
    , database{std::move(_dataBase)}
    , scheduler{database->getScheduler()}
    , wordDB{std::dynamic_pointer_cast<database::WordDB_chi>(database->getWordDB())}
    , coloredVocables{std::move(_coloredVocables)}
    , ratings(createInitialRatings(coloredVocables, scheduler, wordDB, database))
    , fontType{context::getFontType(context::FontSize::small, language)}
{
    setup();
}

auto DisplayVocables_chi::createInitialRatings(const std::vector<ColoredVocable>& coloredVocables,
                                               const std::shared_ptr<sr::Scheduler>& scheduler,
                                               const std::shared_ptr<database::WordDB>& wordDB,
                                               std::shared_ptr<sr::DataBase> database) -> std::vector<Rating>
{
    std::vector<Rating> ratings;
    ranges::transform(coloredVocables, std::back_inserter(ratings),
                      [&](const auto& coloredVocable) -> Rating {
                          const auto& [vocId, _] = coloredVocable;
                          const auto& word = wordDB->lookupId_baseWord(vocId);
                          const auto& srd = *database->Vocables().at_id(vocId).second.SpacedRepetitionData();

                          return scheduler->getRatingSuggestion(srd);
                      });
    return ratings;
}

void DisplayVocables_chi::setup()
{
    using namespace widget::layout;
    auto grid = layer->add<widget::Grid>(Align::start, gridCfg, 7,
                                         widget::Grid::Priorities{0.1F, 0.2F, 0.4F, 0.3F, 0.1F, 0.1F, 0.1F});
    setupVocables(*grid);
}

void DisplayVocables_chi::draw()
{
    layer->start();
    auto& grid = layer->next<widget::Grid>();
    drawVocables(grid);
}

void DisplayVocables_chi::reload()
{
    layer->start();
    auto& grid = layer->next<widget::Grid>();
    setupVocables(grid);
}

auto DisplayVocables_chi::getRatedVocables() const -> VocableId_Rating
{
    VocableId_Rating vocIdEase;
    ranges::transform(coloredVocables, ratings, std::inserter(vocIdEase, vocIdEase.begin()),
                      [](const ColoredVocable& coloredVocable, Rating rating) -> std::pair<VocableId, Rating> {
                          return {coloredVocable.vocableId, rating};
                      });

    return vocIdEase;
}

void DisplayVocables_chi::setupVocables(widget::Grid& grid)
{
    using annotation::tokenVectorFromString;
    using context::Image;

    widget::TextTokenSeq::Config ttqConfig;
    ttqConfig.fontType = fontType;
    ttqConfig.wordPadding = 15.F;

    grid.clear();
    auto itEase = ratings.begin();
    for (const auto& [vocId, colorId] : coloredVocables) {
        const auto& word = wordDB->lookupId(vocId);
        auto rating = *itEase++;
        // const auto
        const auto& oldSrd = *database->Vocables().at_id(vocId).second.SpacedRepetitionData();
        const auto& newSrd = scheduler->review(oldSrd, rating);
        spdlog::info("    {}, {}", word->Key(), oldSrd.serialize());
        bool renderKey = true;
        bool renderEase = true;
        bool renderEnabled = true;
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
                    addRatingButtonGroup(grid);
                    grid.add<widget::TextTokenSeq>(Align::start, makeProgressLabel(newSrd, colorId), ttqConfig);
                    renderEase = false;
                } else {
                    grid.add<widget::Separator>(Align::start, 0.F, 0.F);
                    grid.add<widget::Separator>(Align::start, 0.F, 0.F);
                }
                if (renderEnabled) {
                    grid.add<widget::ImageButton>(Align::start, widget::Images{Image::checkbox,
                                                                               Image::checkbox_checked});
                    grid.add<widget::TextTokenSeq>(Align::start, makeCountLabel(vocId, colorId), ttqConfig);
                    renderEnabled = false;

                } else {
                    grid.add<widget::Separator>(Align::start, 0.F, 0.F);
                    grid.add<widget::Separator>(Align::start, 0.F, 0.F);
                }
            }
        }
    }
}

void DisplayVocables_chi::drawVocables(widget::Grid& grid)
{
    using annotation::tokenVectorFromString;
    using context::Image;

    grid.start();
    auto itEase = ratings.begin();
    for (const auto& [vocId, colorId] : coloredVocables) {
        const auto& word = wordDB->lookupId(vocId);
        auto& rating = *itEase++;

        bool renderKey = true;
        bool renderEase = true;
        bool renderEnabled = true;
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
                    auto updatedRating = grid.next<widget::ToggleButtonGroup>().Active(rating);
                    auto oldRating = std::exchange(rating, updatedRating);

                    auto& ttq = grid.next<widget::TextTokenSeq>();
                    ttq.draw();
                    if (oldRating != rating) {
                        const auto& srd = scheduler->review(
                                *database->Vocables().at_id(vocId).second.SpacedRepetitionData(),
                                rating);
                        ttq.setParagraph(makeProgressLabel(srd, colorId));
                    }

                    renderEase = false;
                } else {
                    grid.next<widget::Separator>();
                    grid.next<widget::Separator>();
                }
                if (renderEnabled) {
                    const auto& srd = database->Vocables().at_id(vocId).second.SpacedRepetitionData();
                    bool checked = srd->enabled;
                    checked = grid.next<widget::ImageButton>().toggled(checked);
                    grid.next<widget::TextTokenSeq>().draw();
                    renderEnabled = false;
                } else {
                    grid.next<widget::Separator>();
                    grid.next<widget::Separator>();
                }
            }
        }
    }
}

void DisplayVocables_chi::addRatingButtonGroup(widget::Grid& grid)
{
    auto tbg = grid.add<widget::ToggleButtonGroup>(Align::start, widget::Orientation::horizontal,
                                                   std::initializer_list<std::string>{"fail", "pass"});
}

auto DisplayVocables_chi::makeProgressLabel(const SpacedRepetitionData& srd, ColorId colorId) -> std::vector<annotation::Token>
{
    // auto progress = ease.getProgress();
    // const auto& scheduler = database->getScheduler();
    auto progressLabel = srd.getDueInTimeLabel();
    return annotation::tokenVectorFromString(progressLabel, colorId);
}

auto DisplayVocables_chi::makeCountLabel(VocableId vocId, ColorId colorId) const -> std::vector<annotation::Token>
{
    auto count = database->Vocables().at_id(vocId).second.CardIds().size();
    auto countLabel = fmt::format("count: {}", count);
    return annotation::tokenVectorFromString(countLabel, colorId);
}

} // namespace gui
