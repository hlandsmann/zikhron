#pragma once
#include <annotation/Word.h>
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
    constexpr static float maxWidth = 500;

    VocableOverlay(std::shared_ptr<widget::Overlay> overlay, std::shared_ptr<widget::TextToken> token);
    void draw();
    [[nodiscard]] auto shouldClose() const -> bool;

private:
    void setupBox();
    using FontType = context::FontType;
    constexpr static FontType fontType{FontType::chineseSmall};
    std::shared_ptr<widget::Overlay> overlay;
    std::shared_ptr<annotation::Word> word;
    std::weak_ptr<widget::TextToken> token;
};
} // namespace gui
