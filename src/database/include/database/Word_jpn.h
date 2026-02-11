#pragma once
#include "Word.h"

#include <dictionary/Dictionary.h>
#include <dictionary/DictionaryJpn.h>
#include <misc/Identifier.h>

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace database {
class SpacedRepetitionData;

struct Definition_jpn;

class Word_jpn : public Word
{
public:
    Word_jpn(const Word_jpn&) = default;
    Word_jpn(Word_jpn&&) = default;
    ~Word_jpn() override = default;
    auto operator=(const Word_jpn&) -> Word_jpn& = default;
    auto operator=(Word_jpn&&) -> Word_jpn& = default;

    using DicEntry = dictionary::DicEntry_jpn;
    using DicKey = dictionary::DicKey_jpn;
    using DicDef = dictionary::DicDef_jpn;
    Word_jpn(const DicEntry& dicEntry, VocableId vocableId);
    Word_jpn(std::string_view description, VocableId vocableId);
    // Word_jpn(std::vector<dictionary::EntryJpn>&& dictionaryEntries, VocableId vocableId);
    [[nodiscard]] auto serializeOld() const -> std::string;
    [[nodiscard]] auto serialize() const -> std::string override;
    [[nodiscard]] auto getId() const -> VocableId override;
    [[nodiscard]] auto getKey() const -> const DicKey&;
    [[nodiscard]] auto Key() const -> std::string;
    [[nodiscard]] auto ShortKey() const -> std::string;
    [[nodiscard]] auto getSpacedRepetitionData() const -> std::shared_ptr<SpacedRepetitionData> override;
    [[nodiscard]] auto getDefinitions() const -> const std::vector<Definition_jpn>&;
    void setDefinitions(const std::vector<Definition_jpn>& definitions);
    [[nodiscard]] auto isConfigureable() const -> bool;
    [[nodiscard]] auto getDictionaryEntries() const -> const std::vector<DicDef>&;
    [[nodiscard]] auto isModified() const -> bool;

    void setDicEntry_toDelete(const DicEntry& dicEntry)
    {
        dicKey = dicEntry.key;
        dictionaryEntries = dicEntry.definitions;
    }

private:
    void parseDefinitions(std::string_view description);
    VocableId vocableId{};
    std::shared_ptr<SpacedRepetitionData> spacedRepetitionData;

    std::string key;
    std::vector<Definition_jpn> definitions;
    std::vector<DicDef> dictionaryEntries;

    DicKey dicKey;
};

struct Definition_jpn
{
    Definition_jpn() = default;
    Definition_jpn(std::string_view description);
    auto operator==(const Definition_jpn&) const -> bool = default;
    [[nodiscard]] auto serialize() const -> std::string;

    std::vector<std::string> readings;
    std::vector<std::string> meanings;
};

using WordPtr_jpn = std::shared_ptr<Word_jpn>;
} // namespace database
