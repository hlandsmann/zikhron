#include "JieBa.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <cppjieba/Jieba.hpp>
#pragma GCC diagnostic pop

#include <Token.h>
#include <WordDB.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace annotation {
JieBa::JieBa(std::shared_ptr<WordDB> _wordDB)
    : wordDB{std::move(_wordDB)}
    , jieba{std::make_shared<cppjieba::Jieba>(
              dict_path,
              hmm_path,
              user_dict_path,
              idf_path,
              stop_word_path)}
{
}

auto JieBa::split(const std::string& str) const -> std::vector<Token>
{
    std::vector<Token> result;
    std::vector<std::string> splitVector;
    jieba->Cut(str, splitVector, true);
    return result;
}

} // namespace annotation
