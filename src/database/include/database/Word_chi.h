#pragma once
#include "Word.h"

#include <dictionary/Dictionary.h>
#include <dictionary/DictionaryChi.h>
#include <misc/Identifier.h>

#include <memory>
#include <string>
#include <string_view>
#include <vector>
class VocableProgress;

namespace database {
class SpacedRepetitionData;

// struct Definition;

class Word_chi
{
public:
    Word_chi(const Word_chi&) = default;
    Word_chi(Word_chi&&) = default;
    virtual ~Word_chi() = default;
    auto operator=(const Word_chi&) -> Word_chi& = default;
    auto operator=(Word_chi&&) -> Word_chi& = default;

    Word_chi(std::string_view description, VocableId vocableId, const std::shared_ptr<dictionary::Dictionary>& dictionary);
    Word_chi(std::vector<dictionary::Entry>&& dictionaryEntries, VocableId vocableId);
    [[nodiscard]] auto serialize() const -> std::string;
    [[nodiscard]] auto getId() const -> VocableId;
    [[nodiscard]] auto Key() const -> std::string;
    [[nodiscard]] auto getSpacedRepetitionData() const -> std::shared_ptr<SpacedRepetitionData>;
    [[nodiscard]] auto getDefinitions() const -> const std::vector<Definition>&;
    void setDefinitions(const std::vector<Definition>& definitions);
    [[nodiscard]] auto isConfigureable() const -> bool;
    [[nodiscard]] auto getDictionaryEntries() const -> const std::vector<dictionary::Entry>&;
    [[nodiscard]] auto isModified() const -> bool;

private:
    void parseDefinitions(std::string_view description);
    VocableId vocableId{};
    std::shared_ptr<SpacedRepetitionData> spacedRepetitionData;

    std::string key;
    std::vector<Definition> definitions;
    std::vector<dictionary::Entry> dictionaryEntries;
};

// struct Definition
// {
//     Definition() = default;
//     Definition(std::string_view description);
//     auto operator==(const Definition&) const -> bool = default;
//     [[nodiscard]] auto serialize() const -> std::string;
//
//     std::string pronounciation;
//     std::vector<std::string> meanings;
// };

} // namespace database
