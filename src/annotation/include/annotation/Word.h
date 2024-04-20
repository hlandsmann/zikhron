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
    Word(std::shared_ptr<VocableProgress> vocableProgress, VocableId vocId, const std::shared_ptr<ZH_Dictionary>& dictionary);
    Word(const std::string& description, const std::shared_ptr<ZH_Dictionary>& dictionary);
    [[nodiscard]] auto serialize() const -> std::string;

private:
    std::shared_ptr<VocableProgress> vocableProgress;

    std::string key;
    std::string pronounciation;
    std::vector<std::string> meanings;
    std::size_t dictionaryPos{};

    std::vector<ZH_Dictionary::Entry> dictionaryEntries;
};

} // namespace annotation
