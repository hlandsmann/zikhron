#include "VocableOverlay.h"

#include <annotation/Token.h>
#include <annotation/TokenText.h>
#include <annotation/Word.h>
#include <context/Fonts.h>
#include <utils/spdlog.h>
#include <widgets/Button.h>
#include <widgets/Grid.h>
#include <widgets/ImageButton.h>
#include <widgets/Layer.h>
#include <widgets/Overlay.h>
#include <widgets/Separator.h>
#include <widgets/TextToken.h>
#include <widgets/TextTokenSeq.h>

#include <algorithm>
#include <iterator>
#include <memory>
#include <ranges>
#include <utility>
#include <vector>
namespace ranges = std::ranges;

namespace gui {
using namespace widget::layout;
namespace views = std::views;

VocableOverlay::VocableOverlay(std::shared_ptr<widget::Overlay> _overlay, std::shared_ptr<widget::TextToken> _token)
    : overlay{std::move(_overlay)}
    , word{_token->getToken().getWord()}
    , token{std::move(_token)}
    , definitions{word->getDefinitions()}
    , options{optionsFromWord(*word)}
{
    using namespace widget::layout;
    overlay->clear();
    overlay->setFirstDrop();

    auto box = overlay->add<widget::Box>(Align::start, widget::Orientation::vertical);
    box->setName("overlayBox");
    setupBox();
}

auto VocableOverlay::optionsFromWord(const annotation::Word& word) -> std::vector<Option>
{
    std::vector<Option> options;
    for (const auto& entry : word.getDictionaryEntries()) {
        auto optionIt = ranges::find(options, entry.pronounciation, &Option::pronounciation);
        if (optionIt == options.end()) {
            options.push_back({.pronounciation = entry.pronounciation,
                               .meanings = entry.meanings,
                               .checked = std::vector<int>(entry.meanings.size())});
            ranges::sort(options.back().meanings);
            continue;
        }
        optionIt->meanings.insert(optionIt->meanings.end(), entry.meanings.begin(), entry.meanings.end());
        optionIt->checked.resize(optionIt->meanings.size());
    }

    const auto& definitions = word.getDefinitions();
    for (const auto& definition : definitions) {
        auto optionIt = ranges::find(options, definition.pronounciation, &Option::pronounciation);
        if (optionIt == options.end()) {
            continue;
        }
        optionIt->open = true;
        for (const auto& meaning : definition.meanings) {
            auto meaningIt = ranges::find(optionIt->meanings, meaning);
            if (meaningIt == optionIt->meanings.end()) {
                continue;
            }
            auto checkedIt = std::next(optionIt->checked.begin(), std::distance(optionIt->meanings.begin(), meaningIt));
            *checkedIt = 1;
        }
    }

    return options;
}

void VocableOverlay::setupBox()
{
    overlay->start();
    auto& box = overlay->next<widget::Box>();
    box.clear();
    const auto& headerBox = box.add<widget::Box>(Align::start, headerBoxCfg, widget::Orientation::horizontal);
    const auto& definitionGrid = box.add<widget::Grid>(Align::start, definitionGridCfg, 2, widget::Grid::Priorities{0.2F, 0.8F});
    const auto& optionBox = box.add<widget::Box>(Align::start, widget::Orientation::vertical);

    headerBox->setName("headerBox");
    headerBox->setExpandType(width_adapt, height_fixed);
    definitionGrid->setName("definitionGrid");

    setupHeader(*headerBox);
    setupDefinition(*definitionGrid);
}

void VocableOverlay::setupHeader(widget::Box& headerBox)
{
    headerBox.clear();
    headerBox.add<widget::TextTokenSeq>(Align::start, annotation::tokenVectorFromString(word->Key(), {}), ttqConfig);
    headerBox.add<widget::Button>(Align::center, "ok");
    headerBox.add<widget::ImageButton>(Align::end, context::Image::configure);
}

void VocableOverlay::drawHeader(widget::Box& headerBox)
{
    headerBox.start();
    headerBox.next<widget::TextTokenSeq>().draw();
    auto& okBtn = headerBox.next<widget::Button>();
    auto& cfgBtn = headerBox.next<widget::ImageButton>();
    if (!word->isConfigureable()) {
        return;
    }
    if (!definitions.empty() && definitions != word->getDefinitions()) {
        if (okBtn.clicked()) {
            word->setDefinitions(definitions);
            wordWasConfigured = true;
            overlay->close();
        }
    }
    if (cfgBtn.clicked()) {
        cfgBtn.setChecked(!cfgBtn.isChecked());
        showOptions = cfgBtn.isChecked();
        if (showOptions) {
            setupPendingOptions = true;
        }
    }
}

void VocableOverlay::setupDefinition(widget::Grid& definitionGrid)
{
    definitionGrid.clear();
    for (const auto& definition : definitions) {
        definitionGrid.add<widget::TextTokenSeq>(Align::start,
                                                 annotation::tokenVectorFromString(definition.pronounciation, {}),
                                                 ttqConfig);
        bool first = true;
        for (const auto& meaning : definition.meanings) {
            if (!first) {
                definitionGrid.add<widget::Separator>(Align::start, 0.F, 0.F);
            }
            first = false;
            definitionGrid.add<widget::TextTokenSeq>(Align::start,
                                                     annotation::tokenVectorFromString(meaning, {}),
                                                     ttqConfig);
        }
    }
    setupPendingDefinition = false;
}

void VocableOverlay::drawDefinition(widget::Grid& definitionGrid)
{
    definitionGrid.start();

    for (const auto& definition : definitions) {
        definitionGrid.next<widget::TextTokenSeq>().draw();
        bool first = true;
        for (const auto& _ : definition.meanings) {
            if (!first) {
                definitionGrid.next<widget::Separator>();
            }
            first = false;
            definitionGrid.next<widget::TextTokenSeq>().draw();
        }
    }
}

void VocableOverlay::setupOptions(widget::Box& optionBox)
{
    using context::Image;
    optionBox.clear();
    for (const auto& option : options) {
        // spdlog::info("p: {}, o: {}", option.pronounciation, option.open);
        auto& prBox = *optionBox.add<widget::Box>(Align::start, boxCfg, widget::Orientation::horizontal);
        auto& openBtn = *prBox.add<widget::ImageButton>(Align::start, Image::arrow_right, Image::arrow_down);
        prBox.add<widget::TextTokenSeq>(Align::start,
                                        annotation::tokenVectorFromString(option.pronounciation, {}),
                                        ttqConfig);
        openBtn.setOpen(option.open);
        if (!option.open) {
            continue;
        }

        for (const auto& [meaning, checked] : views::zip(option.meanings, option.checked)) {
            auto& meaningBox = *optionBox.add<widget::Box>(Align::start, boxCfg, widget::Orientation::horizontal);
            auto& checkBox = *meaningBox.add<widget::ImageButton>(Align::start, Image::checkbox, Image::checkbox_checked);
            checkBox.setOpen(checked != 0);
            meaningBox.add<widget::TextTokenSeq>(Align::start,
                                                 annotation::tokenVectorFromString(meaning, {}),
                                                 ttqConfig);
        }
    }
    setupPendingOptions = false;
}

void VocableOverlay::drawOptions(widget::Box& optionBox)
{
    optionBox.start();
    for (auto& option : options) {
        // spdlog::info("p: {}, o: {}", option.pronounciation, option.open);
        auto& prBox = optionBox.next<widget::Box>();
        if (option.open) {
            for (auto& checked : option.checked) {
                auto& meaningBox = optionBox.next<widget::Box>();
                meaningBox.start();
                auto oldState = std::exchange(checked, meaningBox.next<widget::ImageButton>().isOpen());
                meaningBox.next<widget::TextTokenSeq>().draw();
                if (oldState != checked) {
                    setupPendingDefinition = true;
                    generateDefinitions();
                }
            }
        }
        prBox.start();
        bool oldState = std::exchange(option.open, prBox.next<widget::ImageButton>().isOpen());
        prBox.next<widget::TextTokenSeq>().draw();

        if (oldState != option.open) {
            setupPendingOptions = true;
        }
    }
}

void VocableOverlay::generateDefinitions()
{
    definitions.clear();
    for (const auto& option : options) {
        if (ranges::none_of(option.checked, [](int checked) { return checked != 0; })) {
            continue;
        }
        auto definition = annotation::Definition();
        definition.pronounciation = option.pronounciation;
        for (const auto& [meaning, checked] : views::zip(option.meanings, option.checked)) {
            if (checked != 0) {
                definition.meanings.push_back(meaning);
            }
        }
        definitions.push_back(definition);
    }
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
    auto& optionBox = box.next<widget::Box>();

    drawHeader(headerBox);
    if (!setupPendingDefinition) {
        drawDefinition(definitionGrid);
    }
    if (showOptions && !setupPendingOptions) {
        drawOptions(optionBox);
    }

    if (setupPendingDefinition) {
        setupDefinition(definitionGrid);
    }

    if (setupPendingOptions) {
        setupOptions(optionBox);
    }
    if (!showOptions && optionBox.numberOfWidgets() != 0) {
        optionBox.clear();
    }
}

auto VocableOverlay::shouldClose() const -> bool
{
    return overlay->shouldClose();
}

auto VocableOverlay::wasConfigured() const -> bool
{
    return wordWasConfigured;
}

} // namespace gui