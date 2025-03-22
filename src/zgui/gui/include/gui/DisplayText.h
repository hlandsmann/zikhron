#pragma once
#include "VocableOverlay.h"

#include <annotation/TokenText.h>
#include <context/ColorSet.h>
#include <context/Fonts.h>
#include <context/WidgetId.h>
#include <misc/Language.h>
#include <spaced_repetition/DataBase.h>
#include <widgets/Layer.h>
#include <widgets/Overlay.h>
#include <widgets/TextToken.h>
#include <widgets/TextTokenSeq.h>
#include <widgets/detail/Widget.h>

#include <memory>
#include <optional>

namespace gui {

class DisplayText
{
    using Align = widget::layout::Align;

public:
    DisplayText(std::shared_ptr<widget::Layer> layer,
                std::shared_ptr<widget::Overlay> overlay,
                std::unique_ptr<annotation::TokenText> tokenText,
                std::shared_ptr<sr::DataBase> database,
                Language language);

    auto draw() -> bool;

private:
    constexpr static float s_border = 16.F;
    constexpr static float s_horizontalPadding = 64.F;
    constexpr static float s_padding = 24.F;
    // constexpr static widget::BoxCfg boxCfg = {.padding = s_padding,
    //                                           .paddingHorizontal = s_horizontalPadding,
    //                                           .paddingVertical = s_padding,
    //                                           .border = s_border};

    auto drawDialogue() -> std::optional<std::shared_ptr<widget::TextToken>>;
    auto drawText() -> std::optional<std::shared_ptr<widget::TextToken>>;
    void setupDialogue();
    void setupText();

    std::shared_ptr<widget::Layer> layer;
    std::shared_ptr<widget::Overlay> overlay;
    std::unique_ptr<annotation::TokenText> tokenText;
    context::WidgetId textWidgetId{};

    std::unique_ptr<VocableOverlay> vocableOverlay;

    Language language;
    widget::TextTokenSeq::Config ttqConfig;
    context::ColorSetId colorSetId;
    std::shared_ptr<sr::DataBase> database;
};

} // namespace gui
