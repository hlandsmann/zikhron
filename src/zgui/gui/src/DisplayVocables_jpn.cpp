#include "DisplayVocables_jpn.h"

#include <annotation/Ease.h>
#include <annotation/Token.h>
#include <annotation/TokenText.h>
#include <context/Fonts.h>
#include <database/Word.h>
#include <database/WordDB.h>
#include <database/WordDB_jpn.h>
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
#include <ranges>
#include <string>
#include <utility>
#include <vector>

namespace ranges = std::ranges;
namespace views = std::ranges::views;

namespace gui {
DisplayVocables_jpn::DisplayVocables_jpn(std::shared_ptr<widget::Layer> _layer,
                                         std::shared_ptr<sr::DataBase> _dataBase,
                                         std::vector<ColoredVocable>&& _coloredVocables,
                                         Language language)
    : layer{std::move(_layer)}
    , database{std::move(_dataBase)}
    , scheduler{database->getScheduler()}
    , wordDB{std::dynamic_pointer_cast<database::WordDB_jpn>(database->getWordDB())}
    , coloredVocables{std::move(_coloredVocables)}
    , ratings(createInitialRatings(coloredVocables, scheduler, database))
    , fontType{context::getFontType(context::FontSize::small, language)}
{
    setup();
}

auto DisplayVocables_jpn::createInitialRatings(const std::vector<ColoredVocable>& coloredVocables,
                                               const std::shared_ptr<sr::Scheduler>& scheduler,
                                               std::shared_ptr<sr::DataBase> database) -> std::vector<Rating>
{
    std::vector<Rating> ratings;
    ranges::transform(coloredVocables, std::back_inserter(ratings),
                      [&](const auto& coloredVocable) -> Rating {
                          const auto& [vocId, _] = coloredVocable;
                          // const auto& word = wordDB->lookupId_baseWord(vocId);
                          const auto& srd = *database->Vocables().at_id(vocId).second.SpacedRepetitionData();

                          return scheduler->getRatingSuggestion(srd);
                      });
    return ratings;
}

void DisplayVocables_jpn::setup()
{
    using namespace widget::layout;
    auto grid = layer->add<widget::Grid>(Align::start, gridCfg, 7,
                                         widget::Grid::Priorities{0.1F, 0.2F, 0.4F, 0.3F, 0.1F, 0.1F, 0.1F});
    setupVocables(*grid);
}

void DisplayVocables_jpn::draw()
{
    layer->start();
    auto& grid = layer->next<widget::Grid>();
    drawVocables(grid);
}

void DisplayVocables_jpn::reload()
{
    layer->start();
    auto& grid = layer->next<widget::Grid>();
    setupVocables(grid);
}

auto DisplayVocables_jpn::getRatedVocables() const -> VocableId_Rating
{
    VocableId_Rating vocIdEase;
    ranges::transform(coloredVocables, ratings, std::inserter(vocIdEase, vocIdEase.begin()),
                      [](const ColoredVocable& coloredVocable, Rating rating) -> std::pair<VocableId, Rating> {
                          return {coloredVocable.vocableId, rating};
                      });

    return vocIdEase;
}

void DisplayVocables_jpn::setupVocables(widget::Grid& grid)
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
                    grid.add<widget::TextTokenSeq>(Align::start, tokenVectorFromString(def.pronounciation.front(), colorId), ttqConfig);
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

void DisplayVocables_jpn::drawVocables(widget::Grid& grid)
{
    using annotation::tokenVectorFromString;
    using context::Image;

    ratingByKeyMoveEmphasis();

    grid.start();
    auto itEase = ratings.begin();
    for (const auto& [index, coloredVocable] : views::enumerate(coloredVocables)) {
        const auto [vocId, colorId] = coloredVocable;
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
                    auto& ratingButtonGroup = grid.next<widget::ToggleButtonGroup>();
                    ratingButtonGroup.setEmphasized(index == emphasizeIndex);
                    auto oldRating = std::exchange(rating,
                                                   ratingButtonGroup.Active(rating));

                    auto updatedRatingByKey = ratingByKeyToggle(static_cast<int>(index));
                    if (updatedRatingByKey != oldRating) {
                        rating = updatedRatingByKey;
                    }

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

void DisplayVocables_jpn::addRatingButtonGroup(widget::Grid& grid)
{
    auto tbg = grid.add<widget::ToggleButtonGroup>(Align::start, widget::Orientation::horizontal,
                                                   std::initializer_list<std::string>{"fail", "pass"});
}

auto DisplayVocables_jpn::makeProgressLabel(const SpacedRepetitionData& srd, ColorId colorId) -> std::vector<annotation::Token>
{
    // auto progress = ease.getProgress();
    // const auto& scheduler = database->getScheduler();
    auto progressLabel = srd.getDueInTimeLabel();
    return annotation::tokenVectorFromString(progressLabel, colorId);
}

auto DisplayVocables_jpn::makeCountLabel(VocableId vocId, ColorId colorId) const -> std::vector<annotation::Token>
{
    auto count = database->Vocables().at_id(vocId).second.CardIds().size();
    auto countLabel = fmt::format("count: {}", count);
    return annotation::tokenVectorFromString(countLabel, colorId);
}

} // namespace gui
