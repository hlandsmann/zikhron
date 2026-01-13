#pragma once
#include "Word.h"

#include <dictionary/Dictionary.h>
#include <dictionary/DictionaryChi.h>
#include <misc/Identifier.h>

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace database {
class SpacedRepetitionData;

struct Definition_chi;

class Word_chi : public Word
{
public:
    Word_chi(const Word_chi&) = default;
    Word_chi(Word_chi&&) = default;
    virtual ~Word_chi() = default;
    auto operator=(const Word_chi&) -> Word_chi& = default;
    auto operator=(Word_chi&&) -> Word_chi& = default;

    Word_chi(std::string_view description, VocableId vocableId, const std::shared_ptr<dictionary::Dictionary>& dictionary);
    Word_chi(std::vector<dictionary::EntryChi>&& dictionaryEntries, VocableId vocableId);
    [[nodiscard]] auto serialize() const -> std::string override;
    [[nodiscard]] auto getId() const -> VocableId override;
    [[nodiscard]] auto Key() const -> std::string;
    [[nodiscard]] auto getSpacedRepetitionData() const -> std::shared_ptr<SpacedRepetitionData> override;
    [[nodiscard]] auto getDefinitions() const -> const std::vector<Definition_chi>&;
    void setDefinitions(const std::vector<Definition_chi>& definitions);
    [[nodiscard]] auto isConfigureable() const -> bool;
    [[nodiscard]] auto getDictionaryEntries() const -> const std::vector<dictionary::EntryChi>&;
    [[nodiscard]] auto isModified() const -> bool;

private:
    void parseDefinitions(std::string_view description);
    VocableId vocableId{};
    std::shared_ptr<SpacedRepetitionData> spacedRepetitionData;

    std::string key;
    std::vector<Definition_chi> definitions;
    std::vector<dictionary::EntryChi> dictionaryEntries;
};

struct Definition_chi
{
    Definition_chi() = default;
    Definition_chi(std::string_view description);
    auto operator==(const Definition_chi&) const -> bool = default;
    [[nodiscard]] auto serialize() const -> std::string;

    std::string pronounciation;
    std::vector<std::string> meanings;
};

} // namespace database
