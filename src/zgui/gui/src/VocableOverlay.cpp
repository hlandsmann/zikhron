#include "VocableOverlay.h"

#include <annotation/Token.h>
#include <annotation/TokenText.h>
#include <context/Fonts.h>
#include <widgets/Grid.h>
#include <widgets/ImageButton.h>
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
    // overlay->setName("vocableOverlay");

    // widget::TextTokenSeq::Config ttqConfig;
    // ttqConfig.fontType = fontType;
    // ttqConfig.padding = 15.F;
    // box->add<widget::TextTokenSeq>(Align::start, annotation::tokenVectorFromString(word->Key(), {}), ttqConfig);
    // auto definitionBox = box->add<widget::Box>(Align::start, widget::Orientation::horizontal);
    // definitionBox->setExpandType(width_expand, height_fixed);
    // definitionBox->add<widget::TextTokenSeq>(Align::start,
    //                                          annotation::tokenVectorFromString(word->getPronounciation(), {}),
    //                                          ttqConfig);
    // definitionBox->add<widget::TextTokenSeq>(Align::start,
    //                                          annotation::tokenVectorFromString(word->getMeanings().front(), {}),
    //                                          ttqConfig);
    auto box = overlay->add<widget::Box>(Align::start, widget::Orientation::vertical);
    box->setBorder(16.F);
    box->setName("overlayBox");
    setupBox();
}

void VocableOverlay::setupBox()
{
    using namespace widget::layout;
    overlay->start();
    auto& box = overlay->next<widget::Box>();
    box.clear();

    const auto& headerBox = box.add<widget::Box>(Align::start, widget::Orientation::horizontal);
    headerBox->setExpandType(width_adapt, height_fixed);
    headerBox->setName("headerBox");

    widget::TextTokenSeq::Config ttqConfig;
    ttqConfig.fontType = fontType;
    ttqConfig.wordPadding = 15.F;
    ttqConfig.border = 0.F;
    headerBox->add<widget::TextTokenSeq>(Align::start, annotation::tokenVectorFromString(word->Key(), {}), ttqConfig);
    headerBox->add<widget::ImageButton>(Align::end, context::Image::configure);
    auto definitionGrid = box.add<widget::Grid>(Align::start, 2, widget::Grid::Priorities{0.2F, 0.8F});
    definitionGrid->setName("definitionGrid");
    definitionGrid->setPadding(32.F);
    definitionGrid->setBorder(32.F);
    definitionGrid->setExpandType(width_fixed, height_fixed);
    definitionGrid->add<widget::TextTokenSeq>(Align::start,
                                              annotation::tokenVectorFromString(word->getPronounciation(), {}),
                                              ttqConfig);
    definitionGrid->add<widget::TextTokenSeq>(Align::start,
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
    auto& headerBox = box.next<widget::Box>();
    headerBox.start();
    headerBox.next<widget::TextTokenSeq>().draw();
    headerBox.next<widget::ImageButton>().clicked();

    auto& definitionBox = box.next<widget::Grid>();
    definitionBox.start();

    definitionBox.next<widget::TextTokenSeq>().draw();
    definitionBox.next<widget::TextTokenSeq>().draw();
}

auto VocableOverlay::shouldClose() const -> bool
{
    return overlay->shouldClose();
}

} // namespace gui
