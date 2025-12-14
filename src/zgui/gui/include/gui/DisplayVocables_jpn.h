#pragma once
#include "DisplayVocables.h"

#include <annotation/Ease.h>
#include <annotation/Token.h>
#include <annotation/TokenText.h>
#include <context/Fonts.h>
#include <database/SpacedRepetitionData.h>
#include <database/WordDB_jpn.h>
#include <database/Word_jpn.h>
#include <dictionary/DictionaryChi.h>
#include <misc/Identifier.h>
#include <misc/Language.h>
#include <spaced_repetition/DataBase.h>
#include <spaced_repetition/Scheduler.h>
#include <widgets/Grid.h>
#include <widgets/Layer.h>

#include <map>
#include <memory>
// #include <utility>
#include <vector>

namespace gui {

class DisplayVocables_jpn : public DisplayVocables
{
    friend class DisplayVocables;
    using Align = widget::layout::Align;
    using ColoredVocable = annotation::ColoredVocable;
    using Word = database::Word_jpn;
    using SpacedRepetitionData = database::SpacedRepetitionData;
    using Rating = sr::Rating;

    using VocableId_Rating = std::map<VocableId, Rating>;

public:
    // using pair_vocId_Rating = std::pair<VocableId, Ease>;
    DisplayVocables_jpn(std::shared_ptr<widget::Layer> layer,
                        std::shared_ptr<sr::DataBase> database,
                        std::vector<ColoredVocable>&& coloredVocables,
                        Language language);

    void draw() override;
    void reload() override;

    [[nodiscard]] auto getRatedVocables() const -> VocableId_Rating override;

private:
    [[nodiscard]] static auto createInitialRatings(const std::vector<ColoredVocable>& ColoredVocable,
                                                   const std::shared_ptr<sr::Scheduler>& scheduler,
                                                   std::shared_ptr<sr::DataBase> database) -> std::vector<Rating>;
    void setup();
    void setupVocables(widget::Grid& grid);
    void drawVocables(widget::Grid& grid);
    static void addRatingButtonGroup(widget::Grid& grid);
    [[nodiscard]] static auto makeProgressLabel(const SpacedRepetitionData& srd, ColorId colorId) -> std::vector<annotation::Token>;
    [[nodiscard]] auto makeCountLabel(VocableId vocId, ColorId colorId) const -> std::vector<annotation::Token>;

    constexpr static widget::BoxCfg gridCfg = {.padding = 0.F,
                                               .paddingHorizontal = 64.F,
                                               .paddingVertical = 16.F,
                                               .border = 16.F};

    std::shared_ptr<widget::Layer> layer;
    std::shared_ptr<sr::DataBase> database;
    std::shared_ptr<sr::Scheduler> scheduler;
    std::shared_ptr<database::WordDB_jpn> wordDB;
    std::vector<ColoredVocable> coloredVocables;
    std::vector<Rating> ratings;
    context::FontType fontType;
    int emphasizeIndex = -1;
};

} // namespace gui
