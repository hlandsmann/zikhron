#include "VocableOverlay.h"

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
    , token{std::move(_token)}
{}

void VocableOverlay::draw()
{
  
}

} // namespace gui
