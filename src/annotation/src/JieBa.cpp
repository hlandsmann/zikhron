#include "JieBa.h"

#include <cppjieba/Jieba.hpp>
#include <memory>
#include <string>
#include <vector>

namespace annotation {
JieBa::JieBa()
    : jieba{std::make_shared<cppjieba::Jieba>(
            dict_path,
            hmm_path,
            user_dict_path,
            idf_path,
            stop_word_path)}
{
}

auto JieBa::split(const std::string& str) const -> std::vector<std::string>
{
    std::vector<std::string> result;
    jieba->Cut(str, result, true);
    return result;
}

} // namespace annotation
