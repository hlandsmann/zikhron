#include "TranslationOverlay.h"

#include <annotation/Token.h>
#include <widgets/Box.h>
#include <widgets/Layer.h>
#include <widgets/Overlay.h>
#include <widgets/TextToken.h>
#include <widgets/TextTokenSeq.h>

#include <memory>
#include <string>
#include <utility>

namespace gui {

TranslationOverlay::TranslationOverlay(std::shared_ptr<widget::Overlay> _overlay, std::string _text)
    : overlay{std::move(_overlay)}
    , text{std::move(_text)}
{
    using namespace widget::layout;
    overlay->clear();
    overlay->setFirstDrop();
    auto box = overlay->add<widget::Box>(Align::start, boxCfg, widget::Orientation::horizontal);
    box->setName("translationOverlayBox");
    box->setExpandType(width_adapt, height_fixed);
    box->add<widget::TextTokenSeq>(Align::start, annotation::tokenVectorFromString(text), ttqConfig);
}

void TranslationOverlay::draw()
{
    auto drop = overlay->dropOverlay(0, -150);
    overlay->start();
    auto& box = overlay->next<widget::Box>();
    box.start();
    box.next<widget::TextTokenSeq>().draw();

    if (overlay->shouldClose()) {
        focusDelay++;
    }
    if (focusDelay > 5) {
        overlay->setFirstDrop();
        focusDelay = 0;
    }
}

auto TranslationOverlay::getText() const -> const std::string&
{
    return text;
}

} // namespace gui
