#pragma once
#include <vector>
#include <string_view>

namespace annotation {

struct IDF
{
    std::string_view key;
    float idf;
};

struct DictFreq
{
    std::string_view key;
    int freq;
    std::string_view tag;
};

class FreqDictionary
{
public:
    FreqDictionary();

private:
    void loadDictionary();
    std::vector<IDF> idf;
    std::vector<DictFreq> dictFreq;
};

} // namespace annotation
