#include <Card.h>
#include <ZH_Tokenizer.h>
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
Card::Card(std::string _filename,
           CardId _id,
           std::shared_ptr<const ZH_Dictionary> _dictionary,
           std::shared_ptr<const AnnotationChoiceMap> _annotationChoices)
    : filename{std::move(_filename)}
    , id{_id}
    , dictionary{std::move(_dictionary)}
    , annotationChoices{std::move(_annotationChoices)} {};

auto Card::getTokenizer() const -> ZH_Tokenizer&
{
    if (not tokenizer.has_value()) {
        tokenizer.emplace(getText(), dictionary, annotationChoices);
    }
    return tokenizer.value();
}

void Card::resetTokenizer()
{
    tokenizer.reset();
}

auto Card::Id() const -> CardId
{
    return id;
}

DialogueCard::DialogueCard(const std::string& _filename,
                           CardId _id,
                           std::shared_ptr<const ZH_Dictionary> _dictionary,
                           std::shared_ptr<const AnnotationChoiceMap> _annotationChoices)
    : Card(_filename, _id, std::move(_dictionary), std::move(_annotationChoices)){};

auto DialogueCard::getTextVector() const -> std::vector<icu::UnicodeString>
{
    std::vector<icu::UnicodeString> textVector;
    textVector.reserve(dialogue.size());
    std::transform(
            dialogue.begin(), dialogue.end(), std::back_inserter(textVector), [](const auto& item) {
                return item.text;
            });
    return textVector;
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

TextCard::TextCard(const std::string& _filename,
                   CardId _id,
                   std::shared_ptr<const ZH_Dictionary> _dictionary,
                   std::shared_ptr<const AnnotationChoiceMap> _annotationChoices)
    : Card(_filename, _id, std::move(_dictionary), std::move(_annotationChoices)){};

auto TextCard::getTextVector() const -> std::vector<icu::UnicodeString>
{
    return {text};
}

auto TextCard::getText() const -> utl::StringU8
{
    return {text};
}
