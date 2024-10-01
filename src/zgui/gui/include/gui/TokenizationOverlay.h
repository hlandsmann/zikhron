#pragma once
#include <annotation/Token.h>
#include <annotation/Tokenizer.h>
#include <annotation/TokenizerChi.h>
#include <context/ColorSet.h>
#include <context/Fonts.h>
#include <misc/Identifier.h>
#include <utils/StringU8.h>
#include <widgets/Box.h>
#include <widgets/Overlay.h>
#include <widgets/TextToken.h>
#include <widgets/TextTokenSeq.h>

#include <memory>
#include <vector>

namespace gui {

class TokenizationOverlay
{
    using Align = widget::layout::Align;

public:
    TokenizationOverlay(std::shared_ptr<widget::Overlay> overlay,
                        std::shared_ptr<widget::TextToken> textToken,
                        std::vector<annotation::Alternative>::const_iterator alternative);

    void draw();
    [[nodiscard]] auto shouldClose() const -> bool;
    [[nodiscard]] auto configured() const -> bool;
    [[nodiscard]] auto getTokenizationChoice() const -> std::vector<utl::StringU8>;

private:
    void setupBox();
    void setupCurrent();
    void drawCurrent();
    void setupAlternatives(widget::Box& box);
    void drawAlternatives(widget::Box& box);

    void addAlternateStrU8Vector(widget::Box& box, std::vector<utl::StringU8> altStrU8Vec);
    void alternateColorsForTokens(std::vector<annotation::Token>& tokens) const;

    context::ColorSetId colorSetId{context::ColorSetId::adjacentAlternate};
    constexpr static widget::TextTokenSeq::Config ttqConfig = {.fontType = context::FontType::chineseBig};
    constexpr static float s_border = 2.F;
    constexpr static widget::BoxCfg boxCfg = {.padding = 0.F,
                                              .paddingHorizontal = 0.F,
                                              .paddingVertical = 0.F,
                                              .border = s_border};

    std::shared_ptr<widget::Overlay> overlay;
    std::shared_ptr<widget::Box> currentBox;
    std::weak_ptr<widget::TextToken> textToken;
    annotation::Alternative alternative;

    std::vector<utl::StringU8> current;
    std::vector<utl::StringU8> choice;
    ColorId colorId{};
    bool wasConfigured = false;
};

} // namespace gui
