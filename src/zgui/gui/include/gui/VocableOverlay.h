#pragma once
#include <context/Fonts.h>
#include <database/Word.h>
#include <misc/Language.h>
#include <spaced_repetition/DataBase.h>
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
    enum class Checkbox : unsigned {
        Unchecked = 0,
        Checked = 1,
    };
    enum class Openbtn : unsigned {
        Hide = 0,
        Show = 1,
    };

    struct Option
    {
        std::string pronounciation;
        std::vector<std::string> meanings;
        std::vector<Checkbox> checked;
        Openbtn open{0};
    };

public:
    VocableOverlay(std::shared_ptr<widget::Overlay> overlay,
                   std::shared_ptr<widget::TextToken> token,
                   std::shared_ptr<sr::DataBase> database,
                   Language language);
    void draw();
    [[nodiscard]] auto shouldClose() const -> bool;
    [[nodiscard]] auto configured() const -> bool;

private:
    [[nodiscard]] static auto optionsFromWord(const database::Word& word) -> std::vector<Option>;

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

    // constexpr static float maxWidth = 650;
    constexpr static float s_border = 8.F;
    constexpr static float s_horizontalPadding = 32.F;
    constexpr static float s_padding = 32.F;
    using FontType = context::FontType;
    widget::TextTokenSeq::Config ttqConfig = {.wordPadding = 10.F,
                                              .border = 0.F};
    constexpr static widget::BoxCfg headerBoxCfg = {.padding = 8.F,
                                                    .paddingHorizontal = 0.F,
                                                    .paddingVertical = 0.F,
                                                    .border = s_border};
    constexpr static widget::BoxCfg failEnableBoxCfg = {.padding = 12.F,
                                                        .paddingHorizontal = 0.F,
                                                        .paddingVertical = 0.F,
                                                        .border = 0.F};
    constexpr static widget::BoxCfg boxCfg = {.padding = s_padding,
                                              .paddingHorizontal = 0.F,
                                              .paddingVertical = 0.F,
                                              .border = s_border};
    constexpr static widget::BoxCfg definitionGridCfg = {.padding = 0.F,
                                                         .paddingHorizontal = s_horizontalPadding,
                                                         .paddingVertical = 16.F,
                                                         .border = s_border};

    std::shared_ptr<widget::Overlay> overlay;
    std::shared_ptr<database::Word> word;
    std::weak_ptr<widget::TextToken> textToken;

    std::vector<database::Definition> definitions;
    std::vector<Option> options;

    bool showOptions{false};
    bool wordWasConfigured{false};

    bool vocableIsEnabled{};
    std::shared_ptr<sr::DataBase> database;
};
} // namespace gui
