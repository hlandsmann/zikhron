#include "JieBa.h"

// #include "FreqDictionary.h"

#include <Token.h>
#include <database/WordDB.h>
#include <dictionary/DictionaryChi.h>
#include <utils/StringU8.h>
#include <utils/format.h>

#include <cassert>
#include <memory>
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

namespace {

// struct JToken
// {
//     utl::StringU8 key;
//     int freq;
//     float idf;
// };
// auto calculateTokenVectorValue(const std::vector<JToken> tokenVector,
//                                const std::vector<std::vector<JToken>>& tokenVectors) -> float
// {
//     const std::vector<JToken>& hiTokenVec = tokenVector;
//     const std::vector<JToken>& loTokenVec = tokenVectors.back();
//     std::vector<float> hiValues;
//     std::vector<float> loValues;
//     float loVal = 0;
//     // float hiVal = 0;
//
//     auto hiTokenIt = tokenVector.begin();
//     auto loTokenIt = loTokenVec.begin();
//     auto hiLen = hiTokenIt->key.length();
//     auto loLen = loTokenIt->key.length();
//     while (true) {
//         auto fHiLen = static_cast<float>(hiLen);
//         auto fLoLen = static_cast<float>(loLen);
//
//         if (loLen < hiLen) {
//             float factor = static_cast<float>(loTokenIt->key.length()) / fLoLen;
//             loVal += static_cast<float>(loTokenIt->freq) * factor;
//
//             hiLen -= loLen;
//
//             loTokenIt++;
//             loLen = loTokenIt->key.length();
//
//             continue;
//         }
//         if (loLen > hiLen) {
//             float factor = fHiLen / fLoLen; // fLoLen is loToken key length
//             loVal += static_cast<float>(loTokenIt->freq) * factor;
//
//             loValues.push_back(loVal);
//             loVal = 0;
//
//             hiTokenIt++;
//             loLen -= hiLen;
//             hiLen = hiTokenIt->key.length();
//             continue;
//         }
//         if (loLen == hiLen) {
//             float factor = static_cast<float>(loTokenIt->key.length()) / fLoLen;
//             loVal += static_cast<float>(loTokenIt->freq) * factor;
//
//             loValues.push_back(loVal);
//             loVal = 0;
//
//             hiTokenIt++;
//             loTokenIt++;
//
//             if (loTokenIt == loTokenVec.end()) {
//                 assert(hiTokenIt == hiTokenVec.end());
//                 break;
//             }
//             loLen = loTokenIt->key.length();
//             hiLen = hiTokenIt->key.length();
//         }
//     }
//     ranges::transform(hiTokenVec, std::back_inserter(hiValues), &JToken::freq);
//     std::vector<float> div;
//     ranges::transform(hiValues, loValues,
//                       std::back_inserter(div),
//                       [](float a, float b) {
//                       // spdlog::info("{} - {}", a, std::max(0.1F, b));
//                       return a / std::max(0.1F, b); });
//     return std::accumulate(div.begin(), div.end(), 0.F, std::plus<>());
// }
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

} // namespace

namespace annotation {
JieBa::JieBa(std::shared_ptr<database::WordDB> _wordDB)
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

auto JieBa::cut(const std::string& text) const -> std::vector<std::string>
{
    std::vector<std::string> splitVector;
    jieba->Cut(text, splitVector, true);
    return splitVector;
}

auto JieBa::cutAll(const std::string& text) const -> std::vector<std::string>
{
    std::vector<std::string> splitVector;
    jieba->CutAll(text, splitVector);
    return splitVector;
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

void JieBa::debug()
{
    // spdlog::info("AllWords size: {}", allWords.size());
    // std::set<std::string> knownWords;
    // std::set<std::string> dicWords;
    // std::set<std::string> unknown;
    // std::set<std::string> rule;
    // std::set<std::string> split;
    // std::set<std::string> nosplit;
    //
    // for (const auto& word : allWords) {
    //     if (wordDB->wordIsKnown(word)) {
    //         knownWords.insert(word);
    //         continue;
    //     }
    //     if (wordDB->lookup(word) != nullptr) {
    //         dicWords.insert(word);
    //         continue;
    //     }
    //     if (!rules.findRule(word).empty()) {
    //         rule.insert(word);
    //         continue;
    //     }
    //     if (!splitFurther(word).empty()) {
    //         split.insert(word);
    //         continue;
    //     }
    //
    //     unknown.insert(word);
    // }
    // for (const auto& s : split) {
    //     for (const auto& w : splitFurther(s)) {
    //         const auto& word = w.key;
    //         if (wordDB->wordIsKnown(word)) {
    //             knownWords.insert(word);
    //             continue;
    //         }
    //         if (wordDB->lookup(word) != nullptr) {
    //             dicWords.insert(word);
    //             continue;
    //         }
    //         if (!rules.findRule(word).empty()) {
    //             rule.insert(word);
    //             continue;
    //         }
    //         unknown.insert(word);
    //     }
    // }
    // // spdlog::info("### Known: {}\n{}\n", knownWords.size(), fmt::join(knownWords, ", "));
    // // spdlog::info("### Dictionary: {}\n{}\n", dicWords.size(), fmt::join(dicWords, ", "));
    // // spdlog::info("### Rules: {}\n{}\n", rule.size(), fmt::join(rule, ", "));
    // // spdlog::info("### Split: {}\n{}\n", split.size(), fmt::join(split, ", "));
    // // spdlog::info("### Unknown: {}\n{}\n", unknown.size(), fmt::join(unknown, ", "));
    // spdlog::info("known: {}, dictionary: {}, rules: {}, split: {},  unknown: {}",
    //              knownWords.size(),
    //              dicWords.size(),
    //              rule.size(),
    //              split.size(),
    //              unknown.size());
    //
    // for (const auto& kn : knownWords) {
    //     if (utl::StringU8(kn).length() > 4) {
    //         spdlog::info("kn: {}", kn);
    //     }
    // }
    // spdlog::info("dic: {}", fmt::join(unknown, ", "));
}

} // namespace annotation
