#pragma once
#include <context/Fonts.h>
#include <widgets/Overlay.h>
#include <widgets/TextTokenSeq.h>
#include <widgets/detail/MetaBox.h>

#include <memory>
#include <string>

namespace gui {
class TranslationOverlay
{
public:
    TranslationOverlay(std::shared_ptr<widget::Overlay> overlay, std::string text);
    void draw();
    [[nodiscard]] auto getText() const -> const std::string&;

private:
    using FontType = context::FontType;
    constexpr static FontType fontType{FontType::chineseSmall};
    constexpr static widget::TextTokenSeq::Config ttqConfig = {.fontType = FontType::chineseSmall,
                                                               .wordPadding = 10.F,
                                                               .border = 0.F};
    constexpr static widget::BoxCfg boxCfg = {.padding = 8.F,
                                              .paddingHorizontal = 0.F,
                                              .paddingVertical = 0.F,
                                              .border = 32.F};
    std::shared_ptr<widget::Overlay> overlay;
    std::string text;

    int focusDelay{0};
    float y{};
};

} // namespace gui
