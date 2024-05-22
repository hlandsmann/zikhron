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
using namespace widget::layout;

VocableOverlay::VocableOverlay(std::shared_ptr<widget::Overlay> _overlay, std::shared_ptr<widget::TextToken> _token)
    : overlay{std::move(_overlay)}
    , word{_token->getToken().getWord()}
    , token{std::move(_token)}
{
    using namespace widget::layout;
    overlay->clear();
    overlay->setFirstDrop();

    auto box = overlay->add<widget::Box>(Align::start, widget::Orientation::vertical);
    box->setName("overlayBox");
    setupBox();
}

void VocableOverlay::setupBox()
{
    overlay->start();
    auto& box = overlay->next<widget::Box>();
    box.clear();
    const auto& headerBox = box.add<widget::Box>(Align::start, headerBoxCfg, widget::Orientation::horizontal);
    const auto& definitionGrid = box.add<widget::Grid>(Align::start, definitionGridCfg, 2, widget::Grid::Priorities{0.2F, 0.8F});
    const auto& optionBox = box.add<widget::Box>(Align::start, headerBoxCfg, widget::Orientation::horizontal);

    headerBox->setName("headerBox");
    headerBox->setExpandType(width_adapt, height_fixed);
    definitionGrid->setName("definitionGrid");

    createHeader(*headerBox);
    createDefinition(*definitionGrid);
}

void VocableOverlay::createHeader(widget::Box& headerBox)
{
    headerBox.clear();
    headerBox.add<widget::TextTokenSeq>(Align::start, annotation::tokenVectorFromString(word->Key(), {}), ttqConfig);
    headerBox.add<widget::ImageButton>(Align::end, context::Image::configure);
}

void VocableOverlay::drawHeader(widget::Box& headerBox)
{
    headerBox.start();
    headerBox.next<widget::TextTokenSeq>().draw();
    auto& cfgBtn = headerBox.next<widget::ImageButton>();
    if (!word->isConfigureable()) {
        return;
    }
    if (cfgBtn.clicked()) {
        cfgBtn.setChecked(!cfgBtn.isChecked());
    }
}

void VocableOverlay::createDefinition(widget::Grid& definitionGrid)
{
    definitionGrid.clear();
    for (const auto& option : word->getOptions()) {
        definitionGrid.add<widget::TextTokenSeq>(Align::start,
                                                 annotation::tokenVectorFromString(option.pronounciation, {}),
                                                 ttqConfig);
        definitionGrid.add<widget::TextTokenSeq>(Align::start,
                                                 annotation::tokenVectorFromString(option.meanings.front(), {}),
                                                 ttqConfig);
    }
}

void VocableOverlay::drawDefinition(widget::Grid& definitionGrid)
{
    definitionGrid.start();

    definitionGrid.next<widget::TextTokenSeq>().draw();
    definitionGrid.next<widget::TextTokenSeq>().draw();
}

void VocableOverlay::createOptions(widget::Box& optionBox)
{
}

void VocableOverlay::drawOptions(widget::Box& optionBox)
{
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
    auto& definitionGrid = box.next<widget::Grid>();

    drawHeader(headerBox);
    drawDefinition(definitionGrid);
}

auto VocableOverlay::shouldClose() const -> bool
{
    return overlay->shouldClose();
}

} // namespace gui
