#include "Word.h"

#include "SpacedRepetitionData.h"

#include <VocableProgress.h>
#include <dictionary/Dictionary.h>
#include <dictionary/DictionaryChi.h>
#include <misc/Identifier.h>
#include <utils/format.h>
#include <utils/string_split.h>

#include <algorithm>
#include <iterator>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace ranges = std::ranges;

namespace database {

Word::Word(std::string_view description, VocableId _vocableId, const std::shared_ptr<dictionary::Dictionary>& dictionary)
    : vocableId{_vocableId}
{
    auto rest = std::string_view{description};
    key = utl::split_front(rest, ';');
    dictionaryEntries = dictionary->entriesFromKey(key);
    spacedRepetitionData = std::make_shared<SpacedRepetitionData>(); // SpacedRepetitionData::from
    *spacedRepetitionData = SpacedRepetitionData::deserialize(utl::split_front(rest, ';'));

    parseDefinitions(rest);

    if (dictionaryEntries.empty()) {
        spdlog::critical("Empty1: {}", key);
    }
    // spdlog::info("{};{};{}", key, dictionaryPos, vocableProgress->serialize());
}

Word::Word(std::vector<dictionary::Entry>&& _dictionaryEntries, VocableId _vocableId)
    : vocableId{_vocableId}
    , dictionaryEntries{std::move(_dictionaryEntries)}
{
    key = dictionaryEntries.front().key;
    auto definition = Definition{};
    definition.pronounciation = dictionaryEntries.front().pronounciation;
    definition.meanings.push_back(dictionaryEntries.front().meanings.front());
    definitions.push_back(definition);
    // vocableProgress = std::make_shared<VocableProgress>(VocableProgress::new_vocable);
    spacedRepetitionData = std::make_shared<SpacedRepetitionData>(); // SpacedRepetitionData::from
    if (dictionaryEntries.empty()) {
        spdlog::critical("Empty2: {}", key);
    }
    // *spacedRepetitionData = SpacedRepetitionData::fromVocableProgress(*vocableProgress);
}

auto Word::serialize() const -> std::string
{
    std::vector<std::string> serializedDefinitions;
    ranges::transform(definitions, std::back_inserter(serializedDefinitions), &Definition::serialize);
    return fmt::format("{};{};{}\\\n", key,
                       spacedRepetitionData->serialize(),
                       fmt::join(serializedDefinitions, "\\"));
}

auto Word::getId() const -> VocableId
{
    return vocableId;
}

auto Word::Key() const -> std::string
{
    return key;
}

auto Word::getSpacedRepetitionData() const -> std::shared_ptr<SpacedRepetitionData>
{
    return spacedRepetitionData;
}

auto Word::getDefinitions() const -> const std::vector<Definition>&
{
    return definitions;
}

void Word::setDefinitions(const std::vector<Definition>& _definitions)
{
    definitions = _definitions;
}

auto Word::isConfigureable() const -> bool
{
    return ((!dictionaryEntries.empty())
            && (dictionaryEntries.front().meanings.size() > 1 || dictionaryEntries.size() > 1));
}

auto Word::getDictionaryEntries() const -> const std::vector<dictionary::Entry>&
{
    return dictionaryEntries;
}

auto Word::isModified() const -> bool
{
    if (dictionaryEntries.empty()) {
        spdlog::critical("Missing entry {}", serialize());
        return false;
    }
    return !(definitions.size() == 1
             && definitions.front().meanings.size() == 1
             && definitions.front().pronounciation == dictionaryEntries.front().pronounciation
             && definitions.front().meanings.front() == dictionaryEntries.front().meanings.front());
}

void Word::parseDefinitions(std::string_view description)
{
    auto rest = std::string_view{description};
    while (true) {
        auto optionSV = utl::split_front(rest, '\\');
        if (optionSV.empty()) {
            break;
        }
        definitions.emplace_back(optionSV);
    }
}

Definition::Definition(std::string_view description)
{
    auto rest = std::string_view{description};
    pronounciation = utl::split_front(rest, ';');
    while (true) {
        auto meaning = std::string{utl::split_front(rest, '/')};
        if (meaning.empty()) {
            break;
        }
        meanings.push_back(meaning);
    }
}

auto Definition::serialize() const -> std::string
{
    return fmt::format("{};{}/", pronounciation, fmt::join(meanings, "/"));
}
} // namespace database
