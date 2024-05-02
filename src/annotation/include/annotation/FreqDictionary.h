#pragma once
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

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
    static constexpr auto dict_in_path = "/home/harmen/src/zikhron/third_party/cppjieba/dict/jieba.dict.utf8";
    static constexpr auto idf_in_path = "/home/harmen/src/zikhron/third_party/cppjieba/dict/idf.utf8";

public:
    FreqDictionary();
    [[nodiscard]] auto getFreq(std::string_view key) const  -> int;
    [[nodiscard]] auto getIdf(std::string_view key) const -> float;

private:
    template<class Frequency>
    [[nodiscard]] auto find(std::string_view key) const
            -> std::optional<Frequency const*>;
    void loadFreq(const std::filesystem::path& filename);
    void loadIdf(const std::filesystem::path& filename);
    std::vector<IDF> idf;
    std::vector<DictFreq> freq;
    std::string idfString;
    std::string freqString;
};

} // namespace annotation
