#pragma once
#include <set>
#include "Token.h"
#include "WordDB.h"

#include <memory>
#include <string>
#include <vector>

namespace cppjieba {
class Jieba;
}

namespace annotation {

class JieBa
{
    static constexpr auto dict_path = "/home/harmen/src/zikhron/third_party/cppjieba/dict/jieba.dict.utf8";
    static constexpr auto hmm_path = "/home/harmen/src/zikhron/third_party/cppjieba/dict/hmm_model.utf8";
    static constexpr auto user_dict_path = "/home/harmen/src/zikhron/third_party/cppjieba/dict/user.dict.utf8";
    static constexpr auto idf_path = "/home/harmen/src/zikhron/third_party/cppjieba/dict/idf.utf8";
    static constexpr auto stop_word_path = "/home/harmen/src/zikhron/third_party/cppjieba/dict/stop_words.utf8";

public:
    JieBa(std::shared_ptr<WordDB> wordDB);

    [[nodiscard]] auto split(const std::string& str) const -> std::vector<Token>;

private:
    std::shared_ptr<WordDB> wordDB;
    std::shared_ptr<cppjieba::Jieba> jieba;
    std::set<std::string> rules;
    std::set<std::string> no_rule;
};

}; // namespace annotation
