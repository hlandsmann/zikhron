#include "VocableOverlay.h"

#include <annotation/Token.h>
#include <annotation/TokenText.h>
#include <annotation/Word.h>
#include <context/Fonts.h>
#include <utils/spdlog.h>
#include <widgets/Grid.h>
#include <widgets/ImageButton.h>
#include <widgets/Layer.h>
#include <widgets/Overlay.h>
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
                               .checked = std::vector<bool>(entry.meanings.size())});
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
            *checkedIt = true;
        }
    }

    for (const auto& option : options) {
        spdlog::info("p: {}, o: {}", option.pronounciation, option.open);
        for (const auto& [meaning, checked] : views::zip(option.meanings, option.checked)) {
            spdlog::info("--- {}, {}", meaning, checked);
        }
    }

    return options;
}

void VocableOverlay::setupBox()
{
    overlay->start();
    auto& box = overlay->next<widget::Box>();
    box.clear();
    const auto& headerBox = box.add<widget::Box>(Align::start, boxCfg, widget::Orientation::horizontal);
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
        definitionGrid.add<widget::TextTokenSeq>(Align::start,
                                                 annotation::tokenVectorFromString(definition.meanings.front(), {}),
                                                 ttqConfig);
    }
    setupPendingDefinition = false;
}

void VocableOverlay::drawDefinition(widget::Grid& definitionGrid)
{
    definitionGrid.start();

    definitionGrid.next<widget::TextTokenSeq>().draw();
    definitionGrid.next<widget::TextTokenSeq>().draw();
}

void VocableOverlay::setupOptions(widget::Box& optionBox)
{
    optionBox.clear();
    for (const auto& option : options) {
        // spdlog::info("p: {}, o: {}", option.pronounciation, option.open);
        auto& prBox = *optionBox.add<widget::Box>(Align::start, boxCfg, widget::Orientation::horizontal);
        prBox.add<widget::TextTokenSeq>(Align::start,
                                        annotation::tokenVectorFromString(option.pronounciation, {}),
                                        ttqConfig);

        // for (const auto& [meaning, checked] : views::zip(option.meanings, option.checked)) {
        //     spdlog::info("--- {}, {}", meaning, checked);
        // }
    }
    setupPendingOptions = false;
}

void VocableOverlay::drawOptions(widget::Box& optionBox)
{
    optionBox.start();
    for (const auto& option : options) {
        // spdlog::info("p: {}, o: {}", option.pronounciation, option.open);
        auto& prBox = optionBox.next<widget::Box>();
        prBox.start();
        prBox.next<widget::TextTokenSeq>().draw();
        // for (const auto& [meaning, checked] : views::zip(option.meanings, option.checked)) {
        //     spdlog::info("--- {}, {}", meaning, checked);
        // }
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

} // namespace gui
