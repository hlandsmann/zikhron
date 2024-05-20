#pragma once
#include <annotation/Word.h>
#include <context/Fonts.h>
#include <widgets/Grid.h>
#include <widgets/Layer.h>
#include <widgets/Overlay.h>
#include <widgets/TextToken.h>
#include <widgets/TextTokenSeq.h>

#include <memory>

namespace gui {

class VocableOverlay
{
public:
    constexpr static float maxWidth = 500;

    VocableOverlay(std::shared_ptr<widget::Overlay> overlay, std::shared_ptr<widget::TextToken> token);
    void draw();
    [[nodiscard]] auto shouldClose() const -> bool;

private:
    constexpr static float s_border = 8.F;
    constexpr static float s_horizontalPadding = 32.F;
    void setupBox();
    using FontType = context::FontType;
    constexpr static FontType fontType{FontType::chineseSmall};
    constexpr static widget::TextTokenSeq::Config ttqConfig = {.fontType = FontType::chineseSmall,
                                                               .wordPadding = 15.F,
                                                               .border = 0.F};
    constexpr static widget::BoxCfg headerBoxCfg = {.padding = 0.F,
                                                    .paddingHorizontal = 0.F,
                                                    .paddingVertical = 0.F,
                                                    .border = s_border};
    constexpr static widget::BoxCfg definitionGridCfg = {.padding = 0.F,
                                                         .paddingHorizontal = s_horizontalPadding,
                                                         .paddingVertical = 0.F,
                                                         .border = s_border};

    std::shared_ptr<widget::Overlay> overlay;
    std::shared_ptr<annotation::Word> word;
    std::weak_ptr<widget::TextToken> token;
};
} // namespace gui
