#pragma once
#include <annotation/Ease.h>
#include <annotation/Token.h>
#include <annotation/TokenText.h>
#include <context/Fonts.h>
#include <database/Word.h>
#include <database/WordDB.h>
#include <dictionary/DictionaryChi.h>
#include <misc/Identifier.h>
#include <widgets/Grid.h>
#include <widgets/Layer.h>

#include <map>
#include <memory>
#include <utility>
#include <vector>

namespace gui {

class DisplayVocables
{
    using Align = widget::layout::Align;
    using ActiveVocable = annotation::ActiveVocable;
    using Word = database::Word;

    using VocableId_Ease = std::map<VocableId, Ease>;

public:
    using pair_vocId_Ease = std::pair<VocableId, Ease>;
    DisplayVocables(std::shared_ptr<widget::Layer> layer,
                    std::shared_ptr<database::WordDB> wordDB,
                    std::vector<ActiveVocable>&& orderedVocId_ease);

    void draw();
    void reload();

    [[nodiscard]] auto getVocIdEase() const -> VocableId_Ease;

private:
    void setup();
    void setupVocables(widget::Grid& grid);
    void drawVocables(widget::Grid& grid);
    static void addEaseButtonGroup(widget::Grid& grid);
    static auto makeEaseLabel(const Ease& ease, ColorId colorId) -> std::vector<annotation::Token>;

    constexpr static widget::BoxCfg gridCfg = {.padding = 0.F,
                                               .paddingHorizontal = 64.F,
                                               .paddingVertical = 16.F,
                                               .border = 16.F};

    context::FontType fontType = context::FontType::chineseSmall;

    std::shared_ptr<widget::Layer> layer;
    std::shared_ptr<database::WordDB> wordDB;
    std::vector<ActiveVocable> activeVocables;
};

} // namespace gui
