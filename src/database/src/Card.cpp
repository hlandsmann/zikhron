#include "Card.h"

#include <annotation/Token.h>
#include <annotation/Tokenizer.h>
#include <database/WordDB.h>
#include <dictionary/ZH_Dictionary.h>
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
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <string_view>
#include <utility>
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
    : packName{cardInit.packName}
    , packId{cardInit.packId}
    , indexInPack{cardInit.indexInPack}
    , wordDB{cardInit.wordDB}
    , tokenizer{cardInit.tokenizer}
{
}

void Card::setCardId(CardId _cardId)
{
    cardId = _cardId;
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

auto Card::getAlternatives() const -> std::vector<annotation::Alternative>
{
    return tokenizer->getAlternatives(getText(), tokens);
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
    tokens = tokenizer->getSplitForChoices(tokenizationChoices, getText(), tokens);
}

void Card::executeTokenizer()
{
    const auto& cardText = getText();
    // spdlog::info("{}: {}", Id(), cardText);
    tokens = tokenizer->split(cardText);
    // for (const auto& token : tokenVector) {
    //     wordDB->lookup(token);
    // }
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

// auto DialogueCard::getTextVector() const -> std::vector<icu::UnicodeString>
// {
//     std::vector<icu::UnicodeString> textVector;
//     textVector.reserve(dialogue.size());
//     std::transform(
//             dialogue.begin(), dialogue.end(), std::back_inserter(textVector), [](const auto& item) {
//                 return item.text;
//             });
//     return textVector;
// }

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

// auto TextCard::getTextVector() const -> std::vector<icu::UnicodeString>
// {
//     return {text};
// }

auto TextCard::getText() const -> utl::StringU8
{
    return {text};
}

auto TextCard::serialize() const -> std::string
{
    // std::string serText = text.findAndReplace("/", "／");
    return fmt::format("{}:{}/", s_prefix, text);
}

} // namespace database
