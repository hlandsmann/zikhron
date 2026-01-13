#include "Word_jpn.h"

#include "SpacedRepetitionData.h"
#include "Word.h"

#include <dictionary/Dictionary.h>
#include <dictionary/DictionaryJpn.h>
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

Word_jpn::Word_jpn(std::string_view description, VocableId _vocableId, const std::shared_ptr<dictionary::Dictionary>& dictionary)
    : vocableId{_vocableId}
{
    auto dictionary_jpn = std::dynamic_pointer_cast<dictionary::DictionaryJpn>(dictionary);

    auto rest = std::string_view{description};
    key = utl::split_front(rest, ';');
    dictionaryEntries = dictionary_jpn->entriesFromKey(key);
    spacedRepetitionData = std::make_shared<SpacedRepetitionData>(); // SpacedRepetitionData::from
    *spacedRepetitionData = SpacedRepetitionData::deserialize(utl::split_front(rest, ';'));

    parseDefinitions(rest);

    if (dictionaryEntries.empty()) {
        spdlog::critical("Empty1: {}", key);
    }
}

Word_jpn::Word_jpn(std::vector<dictionary::EntryJpn>&& _dictionaryEntries, VocableId _vocableId)
    : vocableId{_vocableId}
    , dictionaryEntries{std::move(_dictionaryEntries)}
{
    key = dictionaryEntries.front().key;
    auto definition = Definition_jpn{};
    definition.pronounciation = dictionaryEntries.front().pronounciation;
    definition.meanings.push_back(dictionaryEntries.front().meanings.front());
    definitions.push_back(definition);
    spacedRepetitionData = std::make_shared<SpacedRepetitionData>(); // SpacedRepetitionData::from
    if (dictionaryEntries.empty()) {
        spdlog::critical("Empty2: {}", key);
    }
}

auto Word_jpn::serialize() const -> std::string
{
    std::vector<std::string> serializedDefinitions;
    ranges::transform(definitions, std::back_inserter(serializedDefinitions), &Definition_jpn::serialize);
    return fmt::format("{};{};{}\\\n", key,
                       spacedRepetitionData->serialize(),
                       fmt::join(serializedDefinitions, "\\"));
}

auto Word_jpn::getId() const -> VocableId
{
    return vocableId;
}

auto Word_jpn::Key() const -> std::string
{
    return key;
}

auto Word_jpn::getSpacedRepetitionData() const -> std::shared_ptr<SpacedRepetitionData>
{
    return spacedRepetitionData;
}

auto Word_jpn::getDefinitions() const -> const std::vector<Definition_jpn>&
{
    return definitions;
}

void Word_jpn::setDefinitions(const std::vector<Definition_jpn>& _definitions)
{
    definitions = _definitions;
}

auto Word_jpn::isConfigureable() const -> bool
{
    return ((!dictionaryEntries.empty())
            && (dictionaryEntries.front().meanings.size() > 1 || dictionaryEntries.size() > 1));
}

auto Word_jpn::getDictionaryEntries() const -> const std::vector<dictionary::EntryJpn>&
{
    return dictionaryEntries;
}

auto Word_jpn::isModified() const -> bool
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

void Word_jpn::parseDefinitions(std::string_view description)
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

Definition_jpn::Definition_jpn(std::string_view description)
{
    auto rest = std::string_view{description};

    pronounciation.emplace_back(utl::split_front(rest, ';'));
    while (true) {
        auto meaning = std::string{utl::split_front(rest, '/')};
        if (meaning.empty()) {
            break;
        }
        meanings.push_back(meaning);
    }
}

auto Definition_jpn::serialize() const -> std::string
{
    return fmt::format("{};{}/", pronounciation, fmt::join(meanings, "/"));
}
} // namespace database
