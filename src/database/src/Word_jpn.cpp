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
Word_jpn::Word_jpn(const DicEntry& dicEntry, VocableId _vocableId)
    : vocableId{_vocableId}
    , dictionaryEntries{dicEntry.definitions}
    , dicKey{dicEntry.key}
{
    spacedRepetitionData = std::make_shared<SpacedRepetitionData>();

    const auto& firstDef = dicEntry.definitions.front();
    auto definition = Definition_jpn{};
    definition.readings = {firstDef.readings};
    definition.meanings = {firstDef.meanings.front()};
    definitions.push_back(definition);
}

Word_jpn::Word_jpn(std::string_view description, VocableId _vocableId)
    : vocableId{_vocableId}
{

    auto rest = std::string_view{description};
    // key = utl::split_front(rest, ';');
    dicKey = DicKey::deserialize(utl::split_front(rest, ';'));
    spacedRepetitionData = std::make_shared<SpacedRepetitionData>(); // SpacedRepetitionData::from
    *spacedRepetitionData = SpacedRepetitionData::deserialize(utl::split_front(rest, ';'));

    parseDefinitions(rest);
}

// Word_jpn::Word_jpn(std::vector<dictionary::EntryJpn>&& _dictionaryEntries, VocableId _vocableId)
//     : vocableId{_vocableId}
//     , dictionaryEntries{std::move(_dictionaryEntries)}
// {
//     key = dictionaryEntries.front().key;
//     auto definition = Definition_jpn{};
//     definition.readings = dictionaryEntries.front().pronounciation;
//     definition.meanings.push_back(dictionaryEntries.front().meanings.front());
//     definitions.push_back(definition);
//     spacedRepetitionData = std::make_shared<SpacedRepetitionData>(); // SpacedRepetitionData::from
//     if (dictionaryEntries.empty()) {
//         spdlog::critical("Empty2: {}", key);
//     }
// }

auto Word_jpn::serializeOld() const -> std::string
{
    std::vector<std::string> serializedDefinitions;
    ranges::transform(definitions, std::back_inserter(serializedDefinitions), &Definition_jpn::serialize);
    return fmt::format("{};{};{}\\\n", key,
                       spacedRepetitionData->serialize(),
                       fmt::join(serializedDefinitions, "\\"));
}

auto Word_jpn::serialize() const -> std::string
{
    std::vector<std::string> serializedDefinitions;
    ranges::transform(definitions, std::back_inserter(serializedDefinitions), &Definition_jpn::serialize);
    return fmt::format("{};{};{}\\\n", dicKey.serialize(),
                       spacedRepetitionData->serialize(),
                       fmt::join(serializedDefinitions, "\\"));
}

auto Word_jpn::getId() const -> VocableId
{
    return vocableId;
}

auto Word_jpn::getKey() const -> const DicKey&
{
    return dicKey;
}

auto Word_jpn::Key() const -> std::string
{
    return key;
}

auto Word_jpn::ShortKey() const -> std::string
{
    if (!dicKey.kanji.empty()) {
        return dicKey.kanji;
    }
    if (!dicKey.kanjiNorm.empty()) {
        return dicKey.kanjiNorm;
    }
    return dicKey.reading;
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

auto Word_jpn::getDictionaryEntries() const -> const std::vector<DicDef>&
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
             && definitions.front().readings == dictionaryEntries.front().readings
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
    auto readings_sv = utl::split_front(rest, ';');
    // auto reading = readings_sv;
    // readings.emplace_back(reading);
    while (true) {
        auto reading = utl::split_front(readings_sv, '/');
        if (reading.empty()) {
            break;
        }
        readings.emplace_back(reading);
    }
    while (true) {
        auto meaning = utl::split_front(rest, '/');
        if (meaning.empty()) {
            break;
        }
        meanings.emplace_back(meaning);
    }
}

auto Definition_jpn::serialize() const -> std::string
{
    return fmt::format("{}/;{}/", fmt::join(readings, "/"), fmt::join(meanings, "/"));
}
} // namespace database
