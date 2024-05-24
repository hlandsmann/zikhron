#pragma once
#include <annotation/Word.h>
#include <context/Fonts.h>
#include <widgets/Grid.h>
#include <widgets/Layer.h>
#include <widgets/Overlay.h>
#include <widgets/TextToken.h>
#include <widgets/TextTokenSeq.h>

#include <memory>
#include <string>
#include <vector>

namespace gui {

class VocableOverlay
{
    struct Option
    {
        std::string pronounciation;
        std::vector<std::string> meanings;
        std::vector<int> checked;
        bool open{false};
    };

public:
    constexpr static float maxWidth = 650;

    VocableOverlay(std::shared_ptr<widget::Overlay> overlay, std::shared_ptr<widget::TextToken> token);
    void draw();
    [[nodiscard]] auto shouldClose() const -> bool;

private:
    [[nodiscard]] static auto optionsFromWord(const annotation::Word& word) -> std::vector<Option>;

    void setupBox();
    void setupHeader(widget::Box& headerBox);
    void drawHeader(widget::Box& headerBox);
    void setupDefinition(widget::Grid& definitionGrid);
    void drawDefinition(widget::Grid& definitionGrid);
    void setupOptions(widget::Box& optionBox);
    void drawOptions(widget::Box& optionBox);
    void generateDefinitions();

    bool setupPendingDefinition{true};
    bool setupPendingOptions{false};

    constexpr static float s_border = 8.F;
    constexpr static float s_horizontalPadding = 32.F;
    constexpr static float s_padding = 32.F;
    using FontType = context::FontType;
    constexpr static FontType fontType{FontType::chineseSmall};
    constexpr static widget::TextTokenSeq::Config ttqConfig = {.fontType = FontType::chineseSmall,
                                                               .wordPadding = 10.F,
                                                               .border = 0.F};
    constexpr static widget::BoxCfg headerBoxCfg = {.padding = 8.F,
                                                    .paddingHorizontal = 0.F,
                                                    .paddingVertical = 0.F,
                                                    .border = s_border};
    constexpr static widget::BoxCfg boxCfg = {.padding = s_padding,
                                              .paddingHorizontal = 0.F,
                                              .paddingVertical = 0.F,
                                              .border = s_border};
    constexpr static widget::BoxCfg definitionGridCfg = {.padding = 0.F,
                                                         .paddingHorizontal = s_horizontalPadding,
                                                         .paddingVertical = 16.F,
                                                         .border = s_border};

    std::shared_ptr<widget::Overlay> overlay;
    std::shared_ptr<annotation::Word> word;
    std::weak_ptr<widget::TextToken> token;

    std::vector<annotation::Definition> definitions;
    std::vector<Option> options;

    bool showOptions{false};
};
} // namespace gui
