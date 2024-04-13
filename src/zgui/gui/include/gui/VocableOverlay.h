#pragma once
#include <annotation/TokenText.h>
#include <context/Fonts.h>
#include <widgets/Grid.h>
#include <widgets/Layer.h>
#include <widgets/Overlay.h>
#include <widgets/TextToken.h>

#include <memory>

namespace gui {

class VocableOverlay
{
public:
    VocableOverlay(std::shared_ptr<widget::Overlay> overlay, std::shared_ptr<widget::TextToken> token);
    void draw();

private:
    std::shared_ptr<widget::Overlay> overlay;
    std::weak_ptr<widget::TextToken> token;
};
} // namespace gui
