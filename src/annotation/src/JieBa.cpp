#include "JieBa.h"

#include "ZH_Tokenizer.h"

#include <utils/StringU8.h>
#include <utils/spdlog.h>

#include <iterator>
#include <set>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <cppjieba/Jieba.hpp>
#pragma GCC diagnostic pop

#include <Token.h>
#include <WordDB.h>

#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>
namespace ranges = std::ranges;

namespace annotation {
JieBa::JieBa(std::shared_ptr<WordDB> _wordDB)
    : wordDB{std::move(_wordDB)}
    , jieba{std::make_shared<cppjieba::Jieba>(
              dict_path,
              hmm_path,
              user_dict_path,
              idf_path,
              stop_word_path)}
    , rules{wordDB->getDictionary()}
{
}

auto JieBa::split(const std::string& str) -> std::vector<Token>
{
    std::vector<Token> result;
    std::vector<std::string> splitVector;
    jieba->Cut(str, splitVector, true);

    ranges::copy(splitVector, std::inserter(allWords, allWords.begin()));

    return result;
}

auto isUnique(const std::vector<std::vector<std::vector<int>>>& chunks) -> bool
{
    for (const auto& v1 : chunks) {
        auto m = v1.front().back();
        for (const auto& v2 : v1) {
            if (v2.back() > m) {
                return false;
            }
            m--;
        }
    }
    return true;
}

auto JieBa::splitFurther(const std::string& str) -> bool
{
    std::vector<std::string> splitVector;
    jieba->CutAll(str, splitVector);
    if (ranges::none_of(splitVector, [](const std::string& tk) -> bool {
            auto strU8 = utl::StringU8{tk};
            return strU8.length() > 1 || strU8.front().string().length() > 1;
        })) {
        return false;
    }
    if (splitVector.size() == 1) {
        return false;
    }
    std::vector<std::string> found;
    ranges::copy_if(splitVector, std::back_inserter(found),
                    [this](const std::string key) -> bool { return wordDB->getDictionary()->contains(key); });

    auto tokenizer = ZH_Tokenizer{str, wordDB->getDictionary()};
    std::vector<std::string> tokens;

    ranges::transform(tokenizer.Tokens(), std::back_inserter(tokens),
                      [](const ZH_Tokenizer::Token& token) -> std::string {
                          return token.text;
                      });
    if (isUnique(tokenizer.Chunks())) {
        return true;
    }
    spdlog::info("{}: --- {} --- {} *** {} | {}", str,
                 fmt::join(splitVector, ", "),
                 fmt::join(found, ", "),
                 fmt::join(tokens, ", "),
                 tokenizer.Chunks().size());
    for (const auto& v1 : tokenizer.Chunks()) {
        fmt::print("v1.size: {}\n", v1.size());
        for (const auto& v2 : v1) {
            fmt::print("v2.size: {}, --- {}\n", v2.size(), fmt::join(v2, ", "));
        }
    }

    return true;
}

void JieBa::debug()
{
    spdlog::info("AllWords size: {}", allWords.size());
    std::set<std::string> knownWords;
    std::set<std::string> dicWords;
    std::set<std::string> unknown;
    std::set<std::string> rule;
    std::set<std ::string> split;

    for (const auto& word : allWords) {
        if (wordDB->wordIsKnown(word)) {
            knownWords.insert(word);
            continue;
        }
        if (wordDB->lookup(word) != nullptr) {
            dicWords.insert(word);
            continue;
        }
        if (!rules.findRule(word).empty()) {
            rule.insert(word);
            continue;
        }
        if (splitFurther(word)) {
            split.insert(word);
            continue;
        }

        unknown.insert(word);
    }
    // spdlog::info("### Known: {}\n{}\n", knownWords.size(), fmt::join(knownWords, ", "));
    // spdlog::info("### Dictionary: {}\n{}\n", dicWords.size(), fmt::join(dicWords, ", "));
    // spdlog::info("### Rules: {}\n{}\n", rule.size(), fmt::join(rule, ", "));
    // spdlog::info("### Split: {}\n{}\n", split.size(), fmt::join(split, ", "));
    // spdlog::info("### Unknown: {}\n{}\n", unknown.size(), fmt::join(unknown, ", "));
    spdlog::info("known: {}, dictionary: {}, rules: {}, split: {},  unknown: {}",
                 knownWords.size(),
                 dicWords.size(),
                 rule.size(),
                 split.size(),
                 unknown.size());

    for (const auto& kn : knownWords) {
        if (utl::StringU8(kn).length() > 4) {
            spdlog::info("kn: {}", kn);
        }
    }
}
} // namespace annotation
