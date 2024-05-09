#pragma once
#include <dictionary/ZH_Dictionary.h>
#include <misc/Identifier.h>

#include <cstddef>
#include <memory>
#include <string>
#include <vector>
class VocableProgress;

namespace annotation {

class Word
{
public:
    Word(const std::string& description, VocableId vocableId,  const std::shared_ptr<ZH_Dictionary>& dictionary);
    Word(std::vector<ZH_Dictionary::Entry>&& dictionaryEntries, VocableId vocableId);
    [[nodiscard]] auto serialize() const -> std::string;
    [[nodiscard]] auto getId() const -> VocableId;
    [[nodiscard]] auto Key() const -> std::string;
    [[nodiscard]] auto getProgress() const -> std::shared_ptr<VocableProgress>;
    [[nodiscard]] auto getPronounciation() const -> const std::string&;
    [[nodiscard]] auto getMeanings() const -> const std::vector<std::string>&;

private:
    VocableId vocableId{};
    std::shared_ptr<VocableProgress> vocableProgress;

    std::string key;
    std::string pronounciation;
    std::vector<std::string> meanings;
    std::size_t dictionaryPos{};

    std::vector<ZH_Dictionary::Entry> dictionaryEntries;
};

} // namespace annotation
