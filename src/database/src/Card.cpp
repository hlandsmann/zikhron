#include "Card.h"

#include "CbdFwd.h"

#include <annotation/Token.h>
#include <annotation/Tokenizer.h>
#include <annotation/TokenizerChi.h>
#include <database/WordDB.h>
#include <dictionary/DictionaryChi.h>
#include <fmt/format.h>
#include <misc/Identifier.h>
#include <misc/TokenizationChoice.h>
#include <spdlog/spdlog.h>
#include <unicode/unistr.h>
#include <utils/StringU8.h>
#include <utils/format.h>
#include <utils/string_split.h>

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include <sys/types.h>

namespace database {

auto Card::deserializeCard(std::string_view content, const CardInit& cardInit) -> CardPtr
{
    auto rest = content;
    auto type = utl::split_front(rest, ':');
    if (type == "dlg") {
        return std::make_shared<DialogueCard>(rest, cardInit);
    }
    if (type == "txt") {
        return std::make_shared<TextCard>(rest, cardInit);
    }

    return {};
}

Card::Card(const CardInit& cardInit)
    : cardId{cardInit.cardId}
    , packName{cardInit.packName}
    , packId{cardInit.packId}
    , indexInPack{cardInit.indexInPack}
    , wordDB{cardInit.wordDB}
    , tokenizer{cardInit.tokenizer}
{
}

auto Card::getCardId() const -> CardId
{
    return cardId;
}

auto Card::getTokens() const -> const std::vector<annotation::Token>&
{
    return tokens;
}

auto Card::getWordDB() const -> std::shared_ptr<database::WordDB>
{
    return wordDB;
}

auto Card::getTokenizer() const -> std::shared_ptr<annotation::Tokenizer>
{
    return tokenizer;
}

auto Card::getAlternatives() const -> std::vector<annotation::Alternative>
{
    auto tokenizerChi = std::dynamic_pointer_cast<annotation::TokenizerChi>(tokenizer);
    if (tokenizerChi) {
        return tokenizerChi->getAlternatives(getText(), tokens);
    }
    return {};
}

auto Card::getPackId() const -> PackId
{
    return packId;
}

auto Card::getPackName() const -> std::string
{
    return packName;
}

auto Card::getIndexInPack() const -> std::size_t
{
    return indexInPack;
}

void Card::setTokenizationChoices(const TokenizationChoiceVec& tokenizationChoices)
{
    auto tokenizerChi = std::dynamic_pointer_cast<annotation::TokenizerChi>(tokenizer);
    if (tokenizerChi) {
        tokens = tokenizerChi->getSplitForChoices(tokenizationChoices, getText(), tokens);
    }
}

void Card::setActive(bool _active)
{
    active = _active;
}

auto Card::isActive() const -> bool
{
    return active;
}

auto Card::getTokenizerDebug() const -> std::string
{
    return tokenizerDebug;
}

void Card::executeTokenizer()
{
    const auto& cardText = getText();
    tokens = tokenizer->split(cardText);
    tokenizerDebug = tokenizer->debugString();
}

DialogueCard::DialogueCard(std::string_view content,
                           const CardInit& cardInit)
    : Card{cardInit}
    , dialogue{deserialize(content)}
{
    executeTokenizer();
}

auto DialogueCard::getDialogue() const -> const std::vector<DialogueItem>&
{
    return dialogue;
}

auto DialogueCard::deserialize(std::string_view content) -> std::vector<DialogueItem>
{
    auto rest = content;

    std::vector<DialogueItem> dialogue;
    while (!rest.empty()) {
        dialogue.push_back({.speaker = utl::split_front(rest, '/'),
                            .text = utl::split_front(rest, '/')});
    }
    return dialogue;
}

auto DialogueCard::serialize() const -> std::string
{
    std::string result = fmt::format("{}:", s_prefix);
    for (const auto& dlg : dialogue) {
        result += fmt::format("{}/{}/", dlg.speaker, dlg.text);
    }
    return result;
}

auto DialogueCard::getText() const -> utl::StringU8
{
    utl::StringU8 result;
    for (const auto& monologue : dialogue) {
        result.append(monologue.speaker);
        result.append(std::string("~"));
        result.append(monologue.text);
        result.append(std::string("~"));
    }
    return result;
}

TextCard::TextCard(std::string_view content,
                   const CardInit& cardInit)
    : Card{cardInit}
    , text{deserialize(content)}
{
    executeTokenizer();
}

auto TextCard::deserialize(std::string_view content) -> utl::StringU8
{
    auto rest = content;
    utl::StringU8 text = utl::split_front(rest, '/');
    return text;
}

auto TextCard::getText() const -> utl::StringU8
{
    return {text};
}

auto TextCard::serialize() const -> std::string
{
    return fmt::format("{}:{}/", s_prefix, text);
}

SubtitleCard::SubtitleCard(std::vector<std::string> content,
                           const CardInit& cardInit)
    : Card{cardInit}
    , joinedSubs{utl::stringU8VectorFromStrings(content)}
{
    executeTokenizer();
}

auto SubtitleCard::getText() const -> utl::StringU8
{
    return fmt::format("{}", fmt::join(joinedSubs, " - "));
}

} // namespace database
