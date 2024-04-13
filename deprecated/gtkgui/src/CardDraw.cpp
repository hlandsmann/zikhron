#include "TextDraw.h"
#include "VocableOverlay.h"

#include <CardDraw.h>
#include <DataThread.h>
#include <markup/Markup.h>
#include <misc/Identifier.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <boost/range/combine.hpp>
#include <cstddef>
#include <iterator>
#include <memory>
#include <string>
#include <utility>

namespace ranges = std::ranges;

CardDraw::CardDraw(Gtk::Overlay& overlay_in)
    : overlay(overlay_in)
{
    set_column_spacing(48);
}

void CardDraw::setParagraph(const std::shared_ptr<markup::Paragraph>& paragraph_in)
{
    reset();
    paragraph = paragraph_in;
    int textDrawIndex = 0; // ToDo: use enumerate here
    for (const auto& fragment : paragraph->getFragments()) {
        addTextDraw(textDrawIndex % 2, textDrawIndex / 2, fragment);
        textDrawIndex++;
    }
    setupSignals();
}

void CardDraw::addTextDraw(int column, int row, const std::string& markup)
{
    auto textDraw = std::make_unique<TextDraw>();
    textDraw->setFontSize(textFontSize);
    textDraw->setSpacing(textSpacing);
    textDraw->setText(markup);
    attach(*textDraw, column, row);
    textDrawContainer.push_back(std::move(textDraw));
}

void CardDraw::setupSignals()
{
    int textDrawIndex = 0; // ToDo: use enumerate here
    for (auto& textDraw : textDrawContainer) {
        textDraw->signal_hoverByteIndex([this, textDrawIndex](int byteIndex) {
            if (isAnnotation) {
                mouseHoverAnnotation(textDrawIndex, byteIndex);
            } else {
                mouseHoverStandard(textDrawIndex, byteIndex);
            }
        });
        textDraw->signal_mouseLeave([this, textDrawIndex]() {
            paragraph->undoChange();
            textDrawContainer[textDrawIndex]->update_markup(paragraph->getFragments()[textDrawIndex]);
        });
        textDraw->signal_mouseClickByteIndex([this, textDrawIndex](int byteIndex) {
            paragraph->undoChange();
            textDrawContainer[textDrawIndex]->update_markup(paragraph->getFragments()[textDrawIndex]);
            if (isAnnotation) {
                mouseClickAnnotation(textDrawIndex, byteIndex);
            } else {
                mouseClickStandard(textDrawIndex, byteIndex);
            }
        });
        textDrawIndex++;
    }
}

void CardDraw::mouseHoverAnnotation(int textDrawIndex, int byteIndex)
{
    auto startIndexPos = paragraph->fragmentStartPos(textDrawIndex, paragraph->BytePositions());

    paragraph->undoChange();
    if (byteIndex != -1) {
        paragraph->highlightAnnotationAtPosition(startIndexPos + byteIndex);
    }
    textDrawContainer[textDrawIndex]->update_markup(paragraph->getFragments()[textDrawIndex]);
}

void CardDraw::mouseHoverStandard(int textDrawIndex, int byteIndex)
{
    auto startIndexPos = paragraph->fragmentStartPos(textDrawIndex, paragraph->BytePositions());

    paragraph->undoChange();
    if (byteIndex != -1) {
        paragraph->highlightWordAtPosition(startIndexPos + byteIndex, paragraph->BytePositions());
    }
    textDrawContainer[textDrawIndex]->update_markup(paragraph->getFragments()[textDrawIndex]);
}

void CardDraw::mouseClickAnnotation(int textDrawIndex, int byteIndex)
{
    auto startIndexPos = paragraph->fragmentStartPos(textDrawIndex, paragraph->BytePositions());

    auto [activeChoice, marked, unmarked, newPos, combinations, characterSequence] =
            paragraph->getAnnotationPossibilities(byteIndex + startIndexPos);
    if (marked.empty()) {
        return;
    }

    auto rect = textDrawContainer[textDrawIndex]->getCharacterPosition(int(newPos - startIndexPos));
    auto annotationOverlayInit = AnnotationOverlayInit{
            .activeChoice = activeChoice,
            .marked = marked,
            .unmarked = unmarked,
            .x = rect.get_x(),
            .y = rect.get_y(),
            .x_max = overlay.get_size(Gtk::Orientation::HORIZONTAL),
            .y_max = overlay.get_size(Gtk::Orientation::VERTICAL)};
    annotationOverlay = std::make_unique<AnnotationOverlay>(annotationOverlayInit);
    annotationOverlay->signal_annotationChoice(
            [this, combinations, characterSequence](std::optional<int> choice) {
                overlay.remove_overlay(*annotationOverlay);

                if (not choice.has_value())
                    return;
                DataThread::get().submitAnnotation(combinations.at(choice.value()), characterSequence);
            });
    overlay.add_overlay(*annotationOverlay);
}

void CardDraw::mouseClickStandard(int textDrawIndex, int byteIndex)
{
    auto startIndexPos = paragraph->fragmentStartPos(textDrawIndex, paragraph->BytePositions());
    const auto clickedItem = paragraph->wordFromPosition(startIndexPos + byteIndex,
                                                         paragraph->BytePositions());
    const auto vocableChoice = paragraph->getVocableChoiceFromPosition(startIndexPos + byteIndex,
                                                                       paragraph->BytePositions());
    if (clickedItem.empty()) {
        return;
    }
    spdlog::info("{}", clickedItem.front().key);
    for (const auto& word : clickedItem) {
        spdlog::info("    {}, id: {}", word.pronounciation, word.id);
        spdlog::info("        {}", word.meanings.front());
    }

    auto vocableChoiceIt = ranges::find(clickedItem, vocableChoice.id, &ZH_Dictionary::Entry::id);
    if (vocableChoiceIt == clickedItem.end()) {
        spdlog::error("Choice for key {} not found!", clickedItem.front().key);
        return;
    }
    size_t choice_entry = std::distance(clickedItem.begin(), vocableChoiceIt);
    auto newPos = paragraph->getWordStartPosition(startIndexPos + byteIndex, paragraph->BytePositions());
    auto rect = textDrawContainer[textDrawIndex]->getCharacterPosition(int(newPos - startIndexPos));

    auto vocableOverlayInit = VocableOverlayInit{.entries = clickedItem,
                                                 .choice_entry = choice_entry,
                                                 .x = rect.get_x(),
                                                 .y = rect.get_y(),
                                                 .x_max = overlay.get_size(Gtk::Orientation::HORIZONTAL),
                                                 .y_max = overlay.get_size(Gtk::Orientation::VERTICAL)};
    vocableOverlay = std::make_unique<VocableOverlay>(vocableOverlayInit);
    vocableOverlay->signal_vocableChoice(
            [this, clickedItem, vocIdOldChoice = vocableChoice.id](std::optional<int> choice) {
                overlay.remove_overlay(*vocableOverlay);
                if (not choice.has_value()) {
                    return;
                }
                VocableId oldVocId = clickedItem.front().id;
                VocableId newVocId = clickedItem[choice.value()].id;
                spdlog::info("Map id {} / {} to {}", oldVocId, vocIdOldChoice, newVocId);
                DataThread::get().submitVocableChoice(oldVocId, newVocId);
            });
    overlay.add_overlay(*vocableOverlay);
}

void CardDraw::reset()
{
    for (const auto& textDraw : textDrawContainer) {
        remove(*textDraw);
    }

    textDrawContainer.clear();
}
