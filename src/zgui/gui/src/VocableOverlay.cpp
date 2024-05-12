#include "VocableOverlay.h"

#include <annotation/Token.h>
#include <annotation/TokenText.h>
#include <context/Fonts.h>
#include <widgets/Layer.h>
#include <widgets/Overlay.h>
#include <widgets/TextToken.h>
#include <widgets/TextTokenSeq.h>

#include <memory>
#include <utility>

namespace gui {
VocableOverlay::VocableOverlay(std::shared_ptr<widget::Overlay> _overlay, std::shared_ptr<widget::TextToken> _token)
    : overlay{std::move(_overlay)}
    , word{_token->getToken().getWord()}
    , token{std::move(_token)}
{
    using namespace widget::layout;
    overlay->clear();
    overlay->setFirstDrop();

    auto box = overlay->add<widget::Box>(Align::start, widget::Orientation::vertical);
    widget::TextTokenSeq::Config ttqConfig;
    ttqConfig.fontType = fontType;
    ttqConfig.padding = 15.F;
    box->add<widget::TextTokenSeq>(Align::start, annotation::tokenVectorFromString(word->Key(), {}), ttqConfig);
    auto definitionBox = box->add<widget::Box>(Align::start, widget::Orientation::horizontal);
    definitionBox->setExpandType(width_expand, height_fixed);
    definitionBox->add<widget::TextTokenSeq>(Align::start,
                                             annotation::tokenVectorFromString(word->getPronounciation(), {}),
                                             ttqConfig);
    definitionBox->add<widget::TextTokenSeq>(Align::start,
                                             annotation::tokenVectorFromString(word->getMeanings().front(), {}),
                                             ttqConfig);
}

void VocableOverlay::draw()
{
    auto ltoken = token.lock();
    if (!ltoken) {
        return;
    }
    auto rect = ltoken->getPositionRect();

    auto drop = overlay->dropOverlay(rect.x, rect.y + rect.height);
    overlay->start();
    auto& box = overlay->next<widget::Box>();
    box.start();
    box.next<widget::TextTokenSeq>().draw();

    auto& definitionBox = box.next<widget::Box>();
    definitionBox.start();

    definitionBox.next<widget::TextTokenSeq>().draw();
    definitionBox.next<widget::TextTokenSeq>().draw();
}

auto VocableOverlay::shouldClose() const -> bool
{
    return overlay->shouldClose();
}

} // namespace gui
