#pragma once
#include <dictionary/ZH_Dictionary.h>

#include <cstddef>
#include <memory>
#include <string>
#include <vector>
class VocableProgress;

namespace annotation {

class Word
{
public:
    Word(std::string key);

private:
    std::string key;
    std::string pronounciation;
    std::vector<std::string> meanings;
    std::size_t dictionaryPos{};

    std::vector<ZH_Dictionary::Entry> dictionaryEntries;
    std::shared_ptr<VocableProgress> vocableProgress;
};

} // namespace annotation
