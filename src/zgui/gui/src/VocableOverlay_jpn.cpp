#include "VocableOverlay_jpn.h"

#include "theme/Sizes.h"

#include <annotation/Token.h>
#include <annotation/TokenText.h>
#include <context/Fonts.h>
#include <database/SpacedRepetitionData.h>
#include <database/Word.h>
#include <database/Word_jpn.h>
#include <dictionary/DictionaryJpn.h>
#include <misc/Identifier.h>
#include <misc/Language.h>
#include <spaced_repetition/DataBase.h>
#include <utils/format.h>
#include <widgets/Button.h>
#include <widgets/Grid.h>
#include <widgets/ImageButton.h>
#include <widgets/Layer.h>
#include <widgets/Overlay.h>
#include <widgets/ScrollArea.h>
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

VocableOverlay_jpn::VocableOverlay_jpn(std::shared_ptr<widget::Overlay> _overlay,
                                       std::shared_ptr<widget::TextToken> _token,
                                       std::shared_ptr<sr::DataBase> _database,
                                       Language language)
    : overlay{std::move(_overlay)}
    , word{std::dynamic_pointer_cast<database::Word_jpn>(_token->getToken().getWord())}
    , textToken{std::move(_token)}
    , definitions{word->getDefinitions()}
    , options{optionsFromWord(*word)}
    , vocableIsEnabled{word->getSpacedRepetitionData()->enabled}
    , database{std::move(_database)}
{
    using namespace widget::layout;

    ttqConfig.fontType = context::getFontType(context::FontSize::small, language);
    overlay->setMaxWidth(Sizes::vocableOverlay);
    overlay->clear();
    overlay->setFirstDrop();

    auto box = overlay->add<widget::Box>(Align::start, widget::Orientation::vertical);
    box->setName("overlayBox");
    setupBox();
}

auto VocableOverlay_jpn::optionsFromWord(const database::Word_jpn& word) -> std::vector<Option>
{
    std::vector<Option> options;
    ranges::transform(word.getDictionaryEntries(), std::back_inserter(options),
                      [](const dictionary::EntryJpn& entry) -> Option {
                          return {.openPronounciation = OpenBtn::Hide,
                                  .pronounciation = entry.pronounciation,
                                  .checkedPronounciation = std::vector<Checkbox>(entry.pronounciation.size()),
                                  .openMeaning = OpenBtn::Hide,
                                  .meanings = entry.meanings,
                                  .checkedMeaning = std::vector<Checkbox>(entry.meanings.size())};
                      });

    const auto& definitions = word.getDefinitions();
    for (const auto& definition : definitions) {
        auto optionIt = ranges::find_if(options, [&definition](const Option& option) -> bool {
            return ranges::includes(option.pronounciation, definition.pronounciation)
                   && ranges::includes(option.meanings, definition.meanings);
        });
        if (optionIt == options.end()) {
            continue;
        }
        optionIt->openPronounciation = OpenBtn::Hide;
        for (const auto& pronounciation : definition.pronounciation) {
            auto pronounciationIt = ranges::find(optionIt->pronounciation, pronounciation);
            if (pronounciationIt == optionIt->pronounciation.end()) {
                continue;
            }
            auto checkedIt = std::next(optionIt->checkedPronounciation.begin(), std::distance(optionIt->pronounciation.begin(), pronounciationIt));
            *checkedIt = Checkbox::Checked;
        }
        optionIt->openMeaning = OpenBtn::Show;
        for (const auto& meaning : definition.meanings) {
            auto meaningIt = ranges::find(optionIt->meanings, meaning);
            if (meaningIt == optionIt->meanings.end()) {
                continue;
            }
            auto checkedIt = std::next(optionIt->checkedMeaning.begin(), std::distance(optionIt->meanings.begin(), meaningIt));
            *checkedIt = Checkbox::Checked;
        }
    }

    return options;
}

void VocableOverlay_jpn::setupBox()
{
    overlay->start();
    auto& box = overlay->next<widget::Box>();
    box.clear();
    const auto& headerBox = box.add<widget::Box>(Align::start, headerBoxCfg, widget::Orientation::horizontal);
    const auto& definitionGrid = box.add<widget::Grid>(Align::start, definitionGridCfg, 2, widget::Grid::Priorities{0.2F, 0.8F});
    auto& optionScrollArea = *box.add<widget::ScrollArea>(Align::start, "vocableScrollArea");
    const auto& optionBox = optionScrollArea.add<widget::Box>(Align::start, widget::Orientation::vertical);

    headerBox->setName("headerBox");
    headerBox->setExpandType(width_adapt, height_fixed);
    definitionGrid->setName("definitionGrid");

    setupHeader(*headerBox);
    setupDefinition(*definitionGrid);
}

void VocableOverlay_jpn::setupHeader(widget::Box& headerBox)
{
    using context::Image;

    headerBox.clear();
    headerBox.add<widget::TextTokenSeq>(Align::start, annotation::tokenVectorFromString(word->Key()), ttqConfig);
    const auto& layer = headerBox.add<widget::Layer>(Align::start);
    layer->setExpandType(width_expand, height_fixed);
    layer->add<widget::Button>(Align::center, "ok");
    const auto& failEnableBox = layer->add<widget::Box>(Align::center, failEnableBoxCfg, widget::Orientation::horizontal);
    failEnableBox->setExpandType(width_fixed, height_fixed);
    failEnableBox->add<widget::ImageButton>(Align::start, widget::Images{Image::checkbox,
                                                                         Image::checkbox_checked});
    failEnableBox->add<widget::Button>(Align::start, "fail");

    headerBox.add<widget::ImageButton>(Align::end, context::Image::configure);
}

void VocableOverlay_jpn::drawHeader(widget::Box& headerBox)
{
    headerBox.start();
    headerBox.next<widget::TextTokenSeq>().draw();
    auto& layer = headerBox.next<widget::Layer>();
    layer.start();
    auto& okBtn = layer.next<widget::Button>();
    auto& failEnableBox = layer.next<widget::Box>();
    failEnableBox.start();
    auto& enabledCheckbox = failEnableBox.next<widget::ImageButton>();
    auto& failBtn = failEnableBox.next<widget::Button>();
    auto& cfgBtn = headerBox.next<widget::ImageButton>();
    if (!definitions.empty() && definitions != word->getDefinitions()) {
        if (okBtn.clicked()) {
            word->setDefinitions(definitions);
            wordWasConfigured = true;
            overlay->close();
        }
    } else {
        const auto _vocableIsEnabled = enabledCheckbox.toggled(vocableIsEnabled);
        if (_vocableIsEnabled != vocableIsEnabled) {
            database->setVocableEnabled(word->getId(), _vocableIsEnabled);
            wordWasConfigured = true;
            overlay->close();
        }
        failBtn.clicked();
    }
    if (word->isConfigureable() && cfgBtn.clicked()) {
        cfgBtn.setChecked(!cfgBtn.isChecked());
        showOptions = cfgBtn.isChecked();
        if (showOptions) {
            setupPendingOptions = true;
        }
    }
}

void VocableOverlay_jpn::setupDefinition(widget::Grid& definitionGrid)
{
    definitionGrid.clear();
    for (const auto& definition : definitions) {
        auto pronounciation = fmt::format("{}", fmt::join(definition.pronounciation, ", "));
        definitionGrid.add<widget::TextTokenSeq>(Align::start,
                                                 annotation::tokenVectorFromString(pronounciation),
                                                 ttqConfig);
        bool first = true;
        for (const auto& meaning : definition.meanings) {
            if (!first) {
                definitionGrid.add<widget::Separator>(Align::start, 0.F, 0.F);
            }
            first = false;
            definitionGrid.add<widget::TextTokenSeq>(Align::start,
                                                     annotation::tokenVectorFromString(meaning),
                                                     ttqConfig);
        }
    }
    setupPendingDefinition = false;
}

void VocableOverlay_jpn::drawDefinition(widget::Grid& definitionGrid)
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

auto VocableOverlay_jpn::pronounciationStrFromOption(const Option& option) -> std::string
{
    std::vector<std::string> pronounciations;
    for (const auto& [pronounciation, checked] : views::zip(option.pronounciation, option.checkedPronounciation)) {
        if (checked == Checkbox::Checked) {
            pronounciations.push_back(pronounciation);
        }
    }
    if (pronounciations.empty() && (option.pronounciation.size() == 1 || option.openPronounciation == OpenBtn::Hide)) {
        pronounciations.push_back(option.pronounciation.front());
    }
    return fmt::format("{}", fmt::join(pronounciations, ", "));
}

void VocableOverlay_jpn::setupOptions(widget::Box& optionBox)
{
    using context::Image;
    optionBox.clear();
    for (const auto& option : options) {
        // spdlog::info("p: {}, o: {}", option.pronounciation, option.open);
        auto& subHeaderBox = *optionBox.add<widget::Box>(Align::start, boxCfg, widget::Orientation::horizontal);
        subHeaderBox.add<widget::ImageButton>(Align::start, widget::Images{Image::arrow_right,
                                                                           Image::arrow_down});
        subHeaderBox.add<widget::TextTokenSeq>(Align::start,
                                               annotation::tokenVectorFromString(pronounciationStrFromOption(option)),
                                               ttqConfig);
        if (option.pronounciation.size() > 1) {
            subHeaderBox.add<widget::ImageButton>(Align::end, widget::Images{Image::arrow_right,
                                                                             Image::arrow_down});
        }
        if (option.openPronounciation == OpenBtn::Show) {
            for (const auto& [pronounciation, checked] : views::zip(option.pronounciation, option.checkedPronounciation)) {
                auto& pronBox = *optionBox.add<widget::Box>(Align::start, boxCfg, widget::Orientation::horizontal);

                pronBox.add<widget::ImageButton>(Align::start, widget::Images{Image::checkbox,
                                                                              Image::checkbox_checked});
                pronBox.add<widget::TextTokenSeq>(Align::start,
                                                  annotation::tokenVectorFromString(pronounciation),
                                                  ttqConfig);
            }
        }
        if (option.openMeaning == OpenBtn::Show) {
            for (const auto& [meaning, checked] : views::zip(option.meanings, option.checkedMeaning)) {
                auto& meaningBox = *optionBox.add<widget::Box>(Align::start, boxCfg, widget::Orientation::horizontal);

                meaningBox.add<widget::ImageButton>(Align::start, widget::Images{Image::checkbox,
                                                                                 Image::checkbox_checked});
                meaningBox.add<widget::TextTokenSeq>(Align::start,
                                                     annotation::tokenVectorFromString(meaning),
                                                     ttqConfig);
            }
        }
    }
    setupPendingOptions = false;
}

void VocableOverlay_jpn::drawOptions(widget::Box& optionBox)
{
    optionBox.start();
    for (auto& option : options) {
        // spdlog::info("p: {}, o: {}", option.pronounciation, option.open);
        auto& subHeaderBox = optionBox.next<widget::Box>();
        if (option.openPronounciation == OpenBtn::Show) {
            for (auto& checked : option.checkedPronounciation) {
                auto& meaningBox = optionBox.next<widget::Box>();
                meaningBox.start();
                // auto oldState = std::exchange(checked, meaningBox.next<widget::ImageButton>().isOpen());
                auto oldState = checked;
                checked = meaningBox.next<widget::ImageButton>().toggled(checked);
                meaningBox.next<widget::TextTokenSeq>().draw();
                if (oldState != checked) {
                    setupPendingDefinition = true;
                    generateDefinitions();
                }
            }
        }
        if (option.openMeaning == OpenBtn::Show) {
            for (auto& checked : option.checkedMeaning) {
                auto& meaningBox = optionBox.next<widget::Box>();
                meaningBox.start();
                // auto oldState = std::exchange(checked, meaningBox.next<widget::ImageButton>().isOpen());
                auto oldState = checked;
                checked = meaningBox.next<widget::ImageButton>().toggled(checked);
                meaningBox.next<widget::TextTokenSeq>().draw();
                if (oldState != checked) {
                    setupPendingDefinition = true;
                    generateDefinitions();
                }
            }
        }
        subHeaderBox.start();
        auto oldStateOpenMeaning = option.openMeaning;
        auto oldStateOpenPronounciation = option.openPronounciation;

        option.openMeaning = subHeaderBox.next<widget::ImageButton>().toggled(option.openMeaning);
        subHeaderBox.next<widget::TextTokenSeq>().draw();
        if (option.pronounciation.size() > 1) {
            option.openPronounciation = subHeaderBox.next<widget::ImageButton>().toggled(option.openPronounciation);
        }

        if (oldStateOpenMeaning != option.openMeaning || oldStateOpenPronounciation != option.openPronounciation) {
            setupPendingOptions = true;
        }
    }
}

void VocableOverlay_jpn::generateDefinitions()
{
    definitions.clear();
    for (const auto& option : options) {
        if (ranges::none_of(option.checkedMeaning, [](Checkbox checked) -> bool { return checked == Checkbox::Checked; })) {
            continue;
        }
        auto definition = database::Definition_jpn();
        for (const auto& [pronounciation, checked] : views::zip(option.pronounciation, option.checkedPronounciation)) {
            if (checked == Checkbox::Checked) {
                definition.pronounciation.push_back(pronounciation);
            }
        }
        if (definition.pronounciation.empty()) {
            definition.pronounciation.push_back(option.pronounciation.front());
        }
        for (const auto& [meaning, checked] : views::zip(option.meanings, option.checkedMeaning)) {
            if (checked == Checkbox::Checked) {
                definition.meanings.push_back(meaning);
            }
        }
        definitions.push_back(definition);
    }
}

void VocableOverlay_jpn::draw()
{
    auto ltoken = textToken.lock();
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
    auto& optionScrollArea = box.next<widget::ScrollArea>();
    optionScrollArea.start();
    auto& optionBox = optionScrollArea.next<widget::Box>();

    drawHeader(headerBox);
    if (!setupPendingDefinition) {
        drawDefinition(definitionGrid);
    }
    if (showOptions && !setupPendingOptions) {
        auto dropScrollArea = optionScrollArea.dropScrollArea();
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

auto VocableOverlay_jpn::shouldClose() const -> bool
{
    return overlay->shouldClose();
}

auto VocableOverlay_jpn::configured() const -> bool
{
    return wordWasConfigured;
}

} // namespace gui
