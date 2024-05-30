#include "Tokenizer.h"

#include "FreqDictionary.h"

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
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace ranges = std::ranges;
namespace views = std::ranges::views;

namespace {
using JToken = annotation::JToken;
using AToken = annotation::AToken;

auto getCandidates(const utl::StringU8& text,
                   const ZH_Dictionary& dict) -> std::vector<std::vector<AToken>>
{
    std::vector<std::vector<AToken>> candidates;
    candidates.reserve(text.length());
    for (int indexBegin = 0; indexBegin < static_cast<int>(text.length()); indexBegin++) {
        const auto span_lower = ZH_Dictionary::Lower_bound(text.substr(indexBegin, 1), dict.Simplified());
        const auto span_now = ZH_Dictionary::Upper_bound(text.substr(indexBegin, 1), span_lower);

        std::vector<AToken> tokens;
        for (int indexEnd = indexBegin + 1; indexEnd <= static_cast<int>(text.length()); indexEnd++) {
            const auto key = text.substr(indexBegin, indexEnd - indexBegin);
            const auto found = ZH_Dictionary::Lower_bound(key, span_now);

            if (found.empty() || found.begin()->key.substr(0, key.length()) != key) {
                break;
            }
            if (key == found.front().key) {
                auto token = AToken{.key = key,
                                    .str = key};
                tokens.push_back(token);
            }
        }
        if (tokens.empty()) {
            int indexEnd = std::min(static_cast<int>(text.length()), indexBegin + 1);
            const auto key = text.substr(indexBegin, indexEnd - indexBegin);
            tokens.push_back({.key = utl::StringU8{},
                              .str = key});
        }
        candidates.push_back(std::move(tokens));
    }

    return candidates;
}

auto getJTokenCandidates(const utl::StringU8& text,
                         const ZH_Dictionary& dict,
                         const annotation::FreqDictionary& freqDic) -> std::vector<std::vector<JToken>>
{
    auto candidates = getCandidates(text, dict);
    std::vector<std::vector<JToken>> jTokenCandidates;
    ranges::transform(candidates, std::back_inserter(jTokenCandidates),
                      [&](const std::vector<AToken>& atokens) -> std::vector<JToken> {
                          std::vector<JToken> jTokenVec;
                          for (const auto& atoken : atokens) {
                              if (atoken.key.empty()) {
                                  break;
                              }
                              jTokenVec.push_back({.key = atoken.key,
                                                   .freq = freqDic.getFreq(std::string{atoken.key}),
                                                   .idf = freqDic.getIdf(std::string{atoken.key})});
                          }
                          return jTokenVec;
                      });
    return jTokenCandidates;
}

auto previousIndex(const std::vector<std::size_t>& currentVec,
                   std::size_t currentIndex,
                   const std::span<const std::vector<AToken>>& tokens) -> std::size_t
{
    std::size_t pi = 0;
    std::size_t approachIndex = 0;
    while (true) {
        if (approachIndex == currentIndex || approachIndex == currentVec.size()) {
            return pi;
        }
        if (tokens[approachIndex].empty() || tokens[approachIndex].front().key.empty()) {
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
               const std::span<const std::vector<AToken>>& tokens) -> std::size_t
{
    std::size_t currentIndex = cvec.size() - 1;
    return previousIndex(cvec, currentIndex, tokens);
}

auto doPseudoPerm(std::vector<std::size_t>& cvec,
                  const std::span<const std::vector<AToken>>& tokens) -> bool
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
                const std::span<const std::vector<AToken>>& tokens) -> bool
{
    std::size_t extend = 0;
    for (const auto& [i, v] : views::enumerate(perm)) {
        auto index = static_cast<std::size_t>(i);
        if (extend > 0) {
            extend--;
            continue;
        }
        if (tokens[index].empty() || tokens[index].front().key.empty()) {
            return false;
        }
        extend = tokens[index][v].key.length() - 1;
    }
    return true;
}

[[nodiscard]] auto genTokenVector(const std::vector<std::size_t> vec,
                                  const std::span<const std::vector<AToken>>& tokens) -> std::vector<AToken>
{
    std::vector<AToken> result;

    std::size_t extend = 0;
    for (const auto& [i, v] : views::enumerate(vec)) {
        auto index = static_cast<std::size_t>(i);
        if (extend > 0) {
            extend--;
            continue;
        }
        if (tokens[index].empty() || tokens[index].front().key.empty()) {
            continue;
        }
        extend = tokens[index][v].key.length() - 1;
        result.push_back(tokens[index][v]);
    }

    return result;
}

auto getAlternativeATokenVector(const std::span<const std::vector<AToken>>& tokens) -> std::vector<std::vector<AToken>>
{
    std::vector<std::vector<AToken>> altATokenVec;
    auto perm = std::vector<std::size_t>(tokens.size(), 0);
    // printv(perm, tokens);
    bool run = true;
    while (run) {
        run = doPseudoPerm(perm, tokens);
        if (verifyPerm(perm, tokens)) {
            altATokenVec.push_back(genTokenVector(perm, tokens));
        }
    }
    ranges::sort(altATokenVec, ranges::less{}, &std::vector<AToken>::size);
    return altATokenVec;
}

auto chooseCombination(const std::span<const std::vector<AToken>>& tokens)
        -> std::vector<AToken>
{
    std::vector<std::vector<AToken>> altATokenVec = getAlternativeATokenVector(tokens);
    // if (combinations.size() > 1) {
    //     for (const auto& v : combinations) {
    //         float val = calculateTokenVectorValue(v, combinations);
    //         for (const auto& t : v) {
    //             fmt::print("{}/", t.key);
    //         }
    //         fmt::print(" ---{}\n", val);
    //     }
    // }
    return altATokenVec.front();
}
} // namespace

namespace annotation {

Tokenizer::Tokenizer(std::shared_ptr<zikhron::Config> _config, std::shared_ptr<WordDB> _wordDB)
    : config{std::move(_config)}
    , wordDB{std::move(_wordDB)}
    , jieba{wordDB}
    , rules{wordDB->getDictionary()}
    , freqDictionary{std::make_shared<FreqDictionary>()}
{}

struct CandidateSplit
{
    utl::StringU8 cover;
    std::vector<std::vector<AToken>> alts;

    [[nodiscard]] auto empty() const -> bool { return alts.empty(); }
};

auto splitCandidates(const std::span<std::vector<AToken>> candidates) -> CandidateSplit
{
    CandidateSplit alt;
    std::size_t maxLen{1};

    for (const auto& candidate : candidates) {
        std::vector<AToken> tokenVec;
        for (const auto& atoken : candidate) {
            maxLen = std::max(maxLen, atoken.str.length());
            tokenVec.push_back(atoken);
        }
        if (!alt.empty() || maxLen > 1) {
            alt.alts.push_back(tokenVec);
        }
        alt.cover += candidate.front().str.front();
        maxLen--;
        if (maxLen == 0) {
            break;
        }
    }

    return alt;
}

void Tokenizer::getAlternatives(const std::string& text)
{
    spdlog::info("{}", text);
    auto candidates = getCandidates({text}, *wordDB->getDictionary());
    for (const auto& ts : candidates) {
        for (const auto& t : ts) {
            fmt::print("{}/", t.key);
        }
        if (ts.empty()) {
            fmt::print("empty");
        }
        fmt::print("\n");
    }
    fmt::print("---------------\n");
    auto first = candidates.begin();
    auto span = std::span(first, candidates.end());
    while (!span.empty()) {
        auto alt = splitCandidates(span);
        std::advance(first, alt.cover.length());
        span = std::span(first, candidates.end());

        if (alt.alts.empty()) {
            continue;
        }
        auto altATokenVec = getAlternativeATokenVector(alt.alts);
        spdlog::info("{}", alt.cover);
        for (const auto& ts : altATokenVec) {
            for (const auto& t : ts) {
                fmt::print("{}/", t.key);
            }
            fmt::print(" s: {}", altATokenVec.size());
            if (ts.empty()) {
                fmt::print("empty");
            }
            fmt::print("\n");
        }
    }
}

auto Tokenizer::joinMissed(const std::vector<Token>& splitVector, const std::string& text) -> std::vector<Token>
{
    std::vector<Token> result;
    auto candidates = getCandidates({text}, *wordDB->getDictionary());
    auto first = candidates.begin();
    auto span = std::span(first, candidates.end());
    int fillUp = 0;
    for (const auto& token : splitVector) {
        span = std::span(first, candidates.end());
        std::advance(first, token.getValue().length());

        if (fillUp != 0) {
            auto len = token.getValue().length();
            fillUp -= static_cast<int>(len);
            if (fillUp < 0) {
                throw std::runtime_error(fmt::format("joinMissed: fillUp: {}, split: {}, text: {}", fillUp, token.string(), text));
            }
            continue;
        }
        if (!token.getWord()) {
            result.push_back(token);
            continue;
        }

        std::vector<std::vector<AToken>> altATokenVec;
        if (auto alt = splitCandidates(span); !alt.alts.empty()) {
            altATokenVec = getAlternativeATokenVector(alt.alts);
        }
        if (altATokenVec.size() == 1) {
            for (const auto& t : altATokenVec.front()) {
                result.emplace_back(t.key, wordDB->lookup(t.key));
                fillUp += static_cast<int>(t.key.length() - token.getValue().length());
            }
        } else {
            result.push_back(token);
        }
    }
    return result;
}

auto Tokenizer::split(const std::string& text) -> std::vector<Token>
{
    std::vector<Token> result;
    std::vector<std::string> splitVector = jieba.cut(text);

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
                              [this](const AToken& token) -> Token {
                                  auto word = wordDB->lookup(token.key);
                                  return {token.key, word};
                              });
            continue;
        }
        result.emplace_back(utl::StringU8{str});
    }
    result = joinMissed(result, text);

    ranges::copy(splitVector, std::inserter(allWords, allWords.begin()));

    return result;
}

auto Tokenizer::splitFurther(const std::string& str) -> std::vector<AToken>
{
    std::vector<std::string> splitVector = jieba.cutAll(str);
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
    // auto candidates = getJTokenCandidates(str, *wordDB->getDictionary(), *freqDictionary);
    auto candidates = getCandidates(str, *wordDB->getDictionary());
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

} // namespace annotation
