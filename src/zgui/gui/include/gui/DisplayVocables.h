#pragma once
#include <annotation/Ease.h>
#include <annotation/TokenText.h>
#include <annotation/Word.h>
#include <annotation/WordDB.h>
#include <context/Fonts.h>
#include <dictionary/ZH_Dictionary.h>
#include <misc/Identifier.h>
#include <widgets/Grid.h>
#include <widgets/Layer.h>

#include <memory>
#include <utility>
#include <vector>

namespace gui {

class DisplayVocables
{
    using Align = widget::layout::Align;
    using ActiveVocable = annotation::ActiveVocable;

public:
    using pair_vocId_Ease = std::pair<VocableId, Ease>;
    DisplayVocables(std::shared_ptr<widget::Layer> layer,
                    std::shared_ptr<annotation::WordDB> wordDB,
                    std::vector<ActiveVocable>&& orderedVocId_ease);

    void draw();

private:
    void setup();
    void addVocable(widget::Grid& grid, const annotation::Word& word, ColorId colorId);
    static void addEaseButtonGroup(widget::Grid& grid);

    context::FontType fontType = context::FontType::chineseSmall;

    std::shared_ptr<widget::Layer> layer;
    std::shared_ptr<annotation::WordDB> wordDB;
    std::vector<ActiveVocable> activeVocables;
};

} // namespace gui
