#pragma once
#include "Rules.h"

#include <dictionary/ZH_Dictionary.h>

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace annotation {
class AdaptJiebaDict
{
public:
    static constexpr auto dict_in_path = "/home/harmen/src/zikhron/third_party/cppjieba/dict/jieba.dict.utf8";
    static constexpr auto idf_in_path = "/home/harmen/src/zikhron/third_party/cppjieba/dict/idf.utf8";
    static constexpr auto dict_out_path = "/home/harmen/src/zikhron/build/jieba.dict.utf8";
    static constexpr auto idf_out_path = "/home/harmen/src/zikhron/build/idf.utf8";
    static constexpr auto user_dict_path = "/home/harmen/src/zikhron/build/user.dict.utf8";

    AdaptJiebaDict(std::shared_ptr<const ZH_Dictionary> dictionary);
    void load(const std::filesystem::path& dictionaryFileIn);
    void save(const std::filesystem::path& dictionaryFileOut);
    void saveUserDict();
    void merge();

private:
    struct Entry
    {
        std::string_view key;
        std::string_view value;
    };

    std::string fileContent;
    std::vector<Entry> output;
    std::vector<Entry> input;
    std::vector<std::string> userDict;
    std::shared_ptr<const ZH_Dictionary> dictionary;
    Rules rules;
};
} // namespace annotation
