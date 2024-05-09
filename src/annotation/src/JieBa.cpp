#include "JieBa.h"

#include "FreqDictionary.h"
#include "ZH_Tokenizer.h"

#include <Token.h>
#include <WordDB.h>
#include <dictionary/ZH_Dictionary.h>
#include <utils/StringU8.h>
#include <utils/spdlog.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <functional>
#include <iterator>
#include <memory>
#include <numeric>
#include <ranges>
#include <set>
#include <span>
#include <string>
#include <utility>
#include <vector>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <cppjieba/Jieba.hpp>
#pragma GCC diagnostic pop

namespace ranges = std::ranges;
namespace views = std::ranges::views;

namespace {
using JToken = annotation::JToken;

auto GetCandidates(const utl::StringU8& text,
                   const ZH_Dictionary& dict,
                   const annotation::FreqDictionary& freqDic) -> std::vector<std::vector<JToken>>
{
    std::vector<std::vector<JToken>> candidates;
    candidates.reserve(text.length());
    for (int indexBegin = 0; indexBegin < static_cast<int>(text.length()); indexBegin++) {
        const auto span_lower = ZH_Dictionary::Lower_bound(text.substr(indexBegin, 1), dict.Simplified());
        const auto span_now = ZH_Dictionary::Upper_bound(text.substr(indexBegin, 1), span_lower);

        std::vector<JToken> tokens;
        for (int indexEnd = indexBegin + 1; indexEnd <= static_cast<int>(text.length()); indexEnd++) {
            const auto key = text.substr(indexBegin, indexEnd - indexBegin);
            const auto found = ZH_Dictionary::Lower_bound(key, span_now);

            if (found.empty() || found.begin()->key.substr(0, key.length()) != key) {
                break;
            }
            if (key == found.front().key) {
                auto token = JToken{.key = key,
                                    .freq = freqDic.getFreq(key),
                                    .idf = freqDic.getIdf(key)};
                tokens.push_back(token);
            }
        }
        candidates.push_back(std::move(tokens));
    }

    return candidates;
}

auto previousIndex(const std::vector<std::size_t>& currentVec,
                   std::size_t currentIndex,
                   const std::span<const std::vector<JToken>>& tokens) -> std::size_t
{
    std::size_t pi = 0;
    std::size_t approachIndex = 0;
    while (true) {
        if (approachIndex == currentIndex || approachIndex == currentVec.size()) {
            return pi;
        }
        if (tokens[approachIndex].empty()) {
            // spdlog::info("empty");
            approachIndex++;
            continue;
        }
        pi = approachIndex;
        // spdlog::warn("pi: {}, ci: {}", pi, currentIndex);
        // spdlog::warn("cv_pi: {}", currentVec[pi]);
        approachIndex += tokens[pi][currentVec[pi]].key.length();
    }
}

auto lastIndex(std::vector<std::size_t>& cvec,
               const std::span<const std::vector<JToken>>& tokens) -> std::size_t
{
    std::size_t currentIndex = cvec.size() - 1;
    return previousIndex(cvec, currentIndex, tokens);
}

auto doPseudoPerm(std::vector<std::size_t>& cvec,
                  const std::span<const std::vector<JToken>>& tokens) -> bool
{
    std::size_t i = lastIndex(cvec, tokens);
    // spdlog::critical("{}", fmt::join(cvec, ", "));
    // spdlog::info("start i: {}", i);
    while (true) {
        if (i != 0 && tokens[i].size() <= 1) {
            i = previousIndex(cvec, i, tokens);
            continue;
        }
        if (cvec[i] == tokens[i].size() - 1) {
            cvec[i] = 0;
            if (i == 0) {
                // spdlog::info("Stop");
                return false;
                break;
            }
            i = previousIndex(cvec, i, tokens);
            continue;
        }
        cvec[i]++;
        break;
    }
    return true;
}

auto verifyPerm(std::vector<std::size_t>& perm,
                const std::span<const std::vector<JToken>>& tokens) -> bool
{
    std::size_t extend = 0;
    for (const auto& [i, v] : views::enumerate(perm)) {
        auto index = static_cast<std::size_t>(i);
        if (extend > 0) {
            extend--;
            continue;
        }
        if (tokens[index].empty()) {
            return false;
        }
        extend = tokens[index][v].key.length() - 1;
    }
    return true;
}

// void printv(const std::vector<std::size_t> vec,
//             const std::span<const std::vector<JToken>>& tokens)
// {
//     fmt::print("printv: ");
//     std::size_t extend = 0;
//     for (const auto& [i, v] : views::enumerate(vec)) {
//         auto index = static_cast<std::size_t>(i);
//         if (extend > 0) {
//             // fmt::print("x");
//             extend--;
//             continue;
//         }
//         if (tokens[index].empty()) {
//             continue;
//         }
//         extend = tokens[index][v].key.length() - 1;
//         fmt::print("{}, ", tokens[index][v].key);
//     }
//     fmt::print("\n");
// }

[[nodiscard]] auto genTokenVector(const std::vector<std::size_t> vec,
                                  const std::span<const std::vector<JToken>>& tokens) -> std::vector<JToken>
{
    std::vector<JToken> result;

    std::size_t extend = 0;
    for (const auto& [i, v] : views::enumerate(vec)) {
        auto index = static_cast<std::size_t>(i);
        if (extend > 0) {
            extend--;
            continue;
        }
        if (tokens[index].empty()) {
            continue;
        }
        extend = tokens[index][v].key.length() - 1;
        result.push_back(tokens[index][v]);
    }

    return result;
}

auto calculateTokenVectorValue(const std::vector<JToken> tokenVector,
                               const std::vector<std::vector<JToken>>& tokenVectors) -> float
{
    const std::vector<JToken>& hiTokenVec = tokenVector;
    const std::vector<JToken>& loTokenVec = tokenVectors.back();
    std::vector<float> hiValues;
    std::vector<float> loValues;
    float loVal = 0;
    // float hiVal = 0;

    auto hiTokenIt = tokenVector.begin();
    auto loTokenIt = loTokenVec.begin();
    auto hiLen = hiTokenIt->key.length();
    auto loLen = loTokenIt->key.length();
    while (true) {
        auto fHiLen = static_cast<float>(hiLen);
        auto fLoLen = static_cast<float>(loLen);

        if (loLen < hiLen) {
            float factor = static_cast<float>(loTokenIt->key.length()) / fLoLen;
            loVal += static_cast<float>(loTokenIt->freq) * factor;

            hiLen -= loLen;

            loTokenIt++;
            loLen = loTokenIt->key.length();

            continue;
        }
        if (loLen > hiLen) {
            float factor = fHiLen / fLoLen; // fLoLen is loToken key length
            loVal += static_cast<float>(loTokenIt->freq) * factor;

            loValues.push_back(loVal);
            loVal = 0;

            hiTokenIt++;
            loLen -= hiLen;
            hiLen = hiTokenIt->key.length();
            continue;
        }
        if (loLen == hiLen) {
            float factor = static_cast<float>(loTokenIt->key.length()) / fLoLen;
            loVal += static_cast<float>(loTokenIt->freq) * factor;

            loValues.push_back(loVal);
            loVal = 0;

            hiTokenIt++;
            loTokenIt++;

            if (loTokenIt == loTokenVec.end()) {
                assert(hiTokenIt == hiTokenVec.end());
                break;
            }
            loLen = loTokenIt->key.length();
            hiLen = hiTokenIt->key.length();
        }
    }
    ranges::transform(hiTokenVec, std::back_inserter(hiValues), &JToken::freq);
    std::vector<float> div;
    ranges::transform(hiValues, loValues,
                      std::back_inserter(div),
                      [](float a, float b) {
                      // spdlog::info("{} - {}", a, std::max(0.1F, b));
                      return a / std::max(0.1F, b); });
    return std::accumulate(div.begin(), div.end(), 0.F, std::plus<>());
}

auto chooseCombination(const std::span<const std::vector<JToken>>& tokens)
        -> std::vector<JToken>
{
    std::vector<std::vector<JToken>> combinations;
    auto perm = std::vector<std::size_t>(tokens.size(), 0);
    // printv(perm, tokens);
    bool run = true;
    while (run) {
        run = doPseudoPerm(perm, tokens);
        if (verifyPerm(perm, tokens)) {
            combinations.push_back(genTokenVector(perm, tokens));
        }
    }
    ranges::sort(combinations, ranges::less{}, &std::vector<JToken>::size);
    // for (const auto& v : combinations) {
    //     float val = calculateTokenVectorValue(v, combinations);
    //     for (const auto& t : v) {
    //         fmt::print("{}/", t.key);
    //     }
    //     fmt::print(" ---{}\n", val);
    // }
    return combinations.front();
}

} // namespace

namespace annotation {
JieBa::JieBa(std::shared_ptr<WordDB> _wordDB)
    : wordDB{std::move(_wordDB)}
    , freqDictionary{std::make_shared<FreqDictionary>()}
    , jieba{std::make_shared<cppjieba::Jieba>(
              dict_path,
              hmm_path,
              user_dict_path,
              idf_path,
              stop_word_path)}
    , rules{wordDB->getDictionary()}
{
}

auto JieBa::split(const std::string& text) -> std::vector<Token>
{
    std::vector<Token> result;
    std::vector<std::string> splitVector;
    jieba->Cut(text, splitVector, true);

    for (const auto& str : splitVector) {
        if (auto word = wordDB->lookup(str)) {
            result.emplace_back(utl::StringU8{str}, word);
            continue;
        }
        if (const auto& rule = rules.findRule(str); !rule.empty()) {
            auto word = wordDB->lookup(rule);
            result.emplace_back(utl::StringU8{str}, word);
            continue;
        }
        if (const auto& tokens = splitFurther(str); !tokens.empty()) {
            ranges::transform(tokens, std::back_inserter(result),
                              [this](const JToken& token) -> Token {
                                  auto word = wordDB->lookup(token.key);
                                  return {token.key, word};
                              });
            continue;
        }
        result.emplace_back(utl::StringU8{str});
    }

    ranges::copy(splitVector, std::inserter(allWords, allWords.begin()));

    return result;
}

// auto isUnique(const std::vector<std::vector<std::vector<int>>>& chunks) -> bool
// {
//     for (const auto& v1 : chunks) {
//         auto m = v1.front().back();
//         for (const auto& v2 : v1) {
//             if (v2.back() > m) {
//                 return false;
//             }
//             m--;
//         }
//     }
//     return true;
// }

auto JieBa::splitFurther(const std::string& str) -> std::vector<JToken>
{
    std::vector<std::string> splitVector;
    jieba->CutAll(str, splitVector);
    if (ranges::none_of(splitVector, [](const std::string& tk) -> bool {
            auto strU8 = utl::StringU8{tk};
            return strU8.length() > 1 || strU8.front().string().length() > 1;
        })) {
        return {};
    }
    if (splitVector.size() == 1) {
        return {};
    }
    // std::vector<std::string> found;
    // ranges::copy_if(splitVector, std::back_inserter(found),
    //                 [this](const std::string key) -> bool { return wordDB->getDictionary()->contains(key); });

    // auto tokenizer = ZH_Tokenizer{str, wordDB->getDictionary()};
    // std::vector<std::string> tokens;

    // ranges::transform(tokenizer.Tokens(), std::back_inserter(tokens),
    //                   [](const ZH_Tokenizer::Token& token) -> std::string {
    //                       return token.text;
    //                   });
    // if (isUnique(tokenizer.Chunks())) {
    //     return true;
    // }
    auto candidates = GetCandidates(str, *wordDB->getDictionary(), *freqDictionary);
    // spdlog::info("{}: --- {} --- {} *** {} | {}", str,
    //              fmt::join(splitVector, ", "),
    //              fmt::join(found, ", "),
    //              fmt::join(tokens, ", "),
    //              tokenizer.Chunks().size());
    // for (const auto& v1 : tokenizer.Chunks()) {
    //     fmt::print("v1.size: ({})\n", v1.size());
    //     for (const auto& v2 : v1) {
    //         fmt::print("v2.size: {}, --- {}\n", v2.size(), fmt::join(v2, ", "));
    //     }
    // }
    // for (const auto& c1 : candidates) {
    //     std::string cd;
    //     for (const auto& c : c1) {
    //         cd += fmt::format("({}:{},{:.2F}) ", c.key, c.freq, c.idf);
    //     }
    //     fmt::print("cd: {}\n", cd);
    // }

    return chooseCombination(candidates);
    // return {};
}

void JieBa::debug()
{
    spdlog::info("AllWords size: {}", allWords.size());
    std::set<std::string> knownWords;
    std::set<std::string> dicWords;
    std::set<std::string> unknown;
    std::set<std::string> rule;
    std::set<std::string> split;
    std::set<std::string> nosplit;

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
        if (!splitFurther(word).empty()) {
            split.insert(word);
            continue;
        }

        unknown.insert(word);
    }
    for (const auto& s : split) {
        for (const auto& w : splitFurther(s)) {
            const auto& word = w.key;
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
            unknown.insert(word);
        }
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
    spdlog::info("dic: {}", fmt::join(unknown, ", "));
}
} // namespace annotation
