#pragma once
#include "Rules.h"

#include <database/WordDB_chi.h>
#include <utils/StringU8.h>

#include <memory>
#include <set>
#include <string>
#include <vector>

namespace cppjieba {
class Jieba;
}

namespace annotation {

// struct JToken
// {
//     utl::StringU8 key;
//     int freq;
//     float idf;
// };

class JieBa
{
    // static constexpr auto dict_path = "/home/harmen/src/zikhron/third_party/cppjieba/dict/jieba.dict.utf8";
    // static constexpr auto idf_path = "/home/harmen/src/zikhron/third_party/cppjieba/dict/idf.utf8";

    static constexpr auto hmm_path = "/home/harmen/src/zikhron/third_party/cppjieba/dict/hmm_model.utf8";
    // static constexpr auto user_dict_path = "/home/harmen/src/zikhron/third_party/cppjieba/dict/user.dict.utf8";
    static constexpr auto stop_word_path = "/home/harmen/src/zikhron/third_party/cppjieba/dict/stop_words.utf8";

    static constexpr auto dict_path = "/home/harmen/src/zikhron/build/jieba.dict.utf8";
    static constexpr auto idf_path = "/home/harmen/src/zikhron/build/idf.utf8";
    static constexpr auto user_dict_path = "/home/harmen/src/zikhron/build/user.dict.utf8";

public:
    JieBa(std::shared_ptr<database::WordDB_chi> wordDB_chi,
          std::shared_ptr<Rules> rules);

    [[nodiscard]] auto cut(const std::string& text) const -> std::vector<std::string>;
    [[nodiscard]] auto cutAll(const std::string& text) const -> std::vector<std::string>;
    void debug();

private:
    std::shared_ptr<database::WordDB_chi> wordDB_chi;
    std::shared_ptr<cppjieba::Jieba> jieba;
    // std::set<std::string> rules;
    // std::set<std::string> no_rule;

    std::set<std::string> allWords;
    std::shared_ptr<Rules> rules;
};

}; // namespace annotation
