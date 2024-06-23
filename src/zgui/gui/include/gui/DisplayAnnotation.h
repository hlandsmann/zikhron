#pragma once
#include "TokenizationOverlay.h"

#include <annotation/TokenText.h>
#include <annotation/Tokenizer.h>
#include <context/ColorSet.h>
#include <context/Fonts.h>
#include <context/WidgetIdGenerator.h>
#include <utils/StringU8.h>
#include <widgets/Layer.h>
#include <widgets/Overlay.h>
#include <widgets/TextToken.h>
#include <widgets/TextTokenSeq.h>

#include <generator>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

namespace gui {
class DisplayAnnotation
{
    using Align = widget::layout::Align;

public:
    DisplayAnnotation(std::shared_ptr<widget::Layer> layer,
                      std::shared_ptr<widget::Overlay> overlay,
                      std::vector<annotation::Alternative> alternatives,
                      std::unique_ptr<annotation::TokenText> tokenText);
    void draw();
    [[nodiscard]] auto getChoice() const -> std::vector<utl::StringU8>;

private:
    constexpr static float s_border = 16.F;
    constexpr static float s_horizontalPadding = 64.F;
    constexpr static float s_padding = 24.F;
    constexpr static widget::BoxCfg boxCfg = {.padding = s_padding,
                                              .paddingHorizontal = s_horizontalPadding,
                                              .paddingVertical = s_padding,
                                              .border = s_border};
    using TokenAlternative = std::pair<std::shared_ptr<widget::TextToken>,
                                       std::vector<annotation::Alternative>::const_iterator>;

    void drawDialogue();
    void drawText();
    void setupDialogue();
    void setupText();
    auto traverseToken() -> std::generator<const std::shared_ptr<widget::TextToken>&>;
    auto alternativeClicked() -> std::optional<TokenAlternative>;

    widget::TextTokenSeq::Config ttqConfig = {.fontType = context::FontType::chineseBig};

    std::shared_ptr<widget::Layer> layer;
    std::shared_ptr<widget::Overlay> overlay;
    std::vector<annotation::Alternative> alternatives;
    std::unique_ptr<annotation::TokenText> tokenText;
    context::WidgetId textWidgetId{};
    context::ColorSetId colorSetId{context::ColorSetId::adjacentAlternate};

    std::unique_ptr<TokenizationOverlay> tokenizationOverlay;
    std::vector<utl::StringU8> choice;
};

} // namespace gui
