#include "Card.h"

#include "Tokenizer.h"

#include <JieBa.h>
#include <Token.h>
#include <WordDB.h>
#include <dictionary/ZH_Dictionary.h>
#include <fmt/format.h>
#include <misc/Identifier.h>
#include <spdlog/spdlog.h>
#include <unicode/unistr.h>
#include <utils/StringU8.h>

#include <algorithm>
#include <ctre.hpp>
#include <iterator>
#include <memory>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <utility>
#include <vector>

#include <sys/types.h>

namespace annotation {

Card::Card(std::string _filename,
           CardId _id,
           std::shared_ptr<WordDB> _wordDB,
           std::shared_ptr<annotation::Tokenizer> _tokenizer)
    : filename{std::move(_filename)}
    , id{_id}
    , wordDB{std::move(_wordDB)}
    , tokenizer{std::move(_tokenizer)} {

    };

auto Card::Id() const -> CardId
{
    return id;
}

auto Card::getTokens() const -> const std::vector<Token>&
{
    return tokens;
}

auto Card::getWordDB() const -> std::shared_ptr<WordDB>
{
    return wordDB;
}

void Card::getAlternatives()
{
    tokenizer->getAlternatives(getText(), tokens);
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

DialogueCard::DialogueCard(std::string _filename,
                           CardId _id,
                           std::shared_ptr<WordDB> _wordDB,
                           std::shared_ptr<annotation::Tokenizer> _tokenizer,
                           std::vector<DialogueItem>&& _dialogue)
    : Card{_filename, _id, std::move(_wordDB), std::move(_tokenizer)}
    , dialogue{std::move(_dialogue)}
{
    executeTokenizer();
};

auto DialogueCard::getDialogue() const -> const std::vector<DialogueItem>&
{
    return dialogue;
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

TextCard::TextCard(std::string _filename,
                   CardId _id,
                   std::shared_ptr<WordDB> _wordDB,
                   std::shared_ptr<annotation::Tokenizer> _tokenizer,
                   icu::UnicodeString _text)
    : Card{_filename, _id, std::move(_wordDB), std::move(_tokenizer)}
    , text{std::move(_text)}
{
    executeTokenizer();
};

// auto TextCard::getTextVector() const -> std::vector<icu::UnicodeString>
// {
//     return {text};
// }

auto TextCard::getText() const -> utl::StringU8
{
    return {text};
}

} // namespace annotation
