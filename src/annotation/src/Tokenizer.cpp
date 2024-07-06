#include "Tokenizer.h"

#include "AnnotationFwd.h"
#include "FreqDictionary.h"

#include <Token.h>
#include <dictionary/WordDB.h>
#include <dictionary/ZH_Dictionary.h>
#include <misc/Config.h>
#include <utils/StringU8.h>
#include <utils/format.h>

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

namespace annotation {
// struct JToken
// {
//     utl::StringU8 key;
//     int freq;
//     float idf;
// };
// using JToken = annotation::JToken;
// auto getJTokenCandidates(const utl::StringU8& text,
//                          const ZH_Dictionary& dict,
//                          const annotation::FreqDictionary& freqDic) -> std::vector<std::vector<JToken>>
// {
//     auto candidates = getCandidates(text, dict);
//     std::vector<std::vector<JToken>> jTokenCandidates;
//     ranges::transform(candidates, std::back_inserter(jTokenCandidates),
//                       [&](const std::vector<AToken>& atokens) -> std::vector<JToken> {
//                           std::vector<JToken> jTokenVec;
//                           for (const auto& atoken : atokens) {
//                               if (atoken.key.empty()) {
//                                   break;
//                               }
//                               jTokenVec.push_back({.key = atoken.key,
//                                                    .freq = freqDic.getFreq(std::string{atoken.key}),
//                                                    .idf = freqDic.getIdf(std::string{atoken.key})});
//                           }
//                           return jTokenVec;
//                       });
//     return jTokenCandidates;
// }

Tokenizer::Tokenizer(std::shared_ptr<zikhron::Config> _config, std::shared_ptr<WordDB> _wordDB)
    : config{std::move(_config)}
    , wordDB{std::move(_wordDB)}
    , jieba{wordDB}
    , rules{wordDB->getDictionary()}
    , freqDictionary{std::make_shared<FreqDictionary>()}
{}

auto Tokenizer::getCandidates(const utl::StringU8& text,
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
            const auto& rule = rules.findRule(key);

            if ((rule.empty() && !rules.approachRule(key))
                && (found.empty() || found.begin()->key.substr(0, key.length()) != key)) {
                break;
            }
            if ((!rule.empty()) || key == found.front().key) {
                auto token = AToken{.key = key,
                                    .str = key};
                tokens.push_back(token);
            }
        }
        if (tokens.empty()) {
            int indexEnd = std::min(static_cast<int>(text.length()), indexBegin + 1);
            const auto key = text.substr(indexBegin, indexEnd - indexBegin);
            tokens.push_back({.key = {},
                              .str = key});
        }
        candidates.push_back(std::move(tokens));
    }

    return candidates;
}

auto Tokenizer::previousIndex(const std::vector<std::size_t>& currentVec,
                              std::size_t currentIndex,
                              std::span<const std::vector<AToken>> tokens) -> std::size_t
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

auto Tokenizer::lastIndex(std::vector<std::size_t>& cvec,
                          std::span<const std::vector<AToken>> tokens) -> std::size_t
{
    std::size_t currentIndex = cvec.size() - 1;
    return previousIndex(cvec, currentIndex, tokens);
}

auto Tokenizer::doPseudoPerm(std::vector<std::size_t>& cvec,
                             std::span<const std::vector<AToken>> tokens) -> bool
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

auto Tokenizer::verifyPerm(std::vector<std::size_t>& perm,
                           std::span<const std::vector<AToken>> tokens) -> bool
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

auto Tokenizer::genTokenVector(const std::vector<std::size_t>& vec,
                               std::span<const std::vector<AToken>> tokens) -> std::vector<AToken>
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

auto Tokenizer::getAlternativeATokenVector(std::span<const std::vector<AToken>> tokens)
        -> std::vector<std::vector<AToken>>
{
    if (tokens.empty()) {
        return {};
    }
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

auto Tokenizer::chooseCombination(std::span<const std::vector<AToken>> tokens)
        -> std::vector<AToken>
{
    std::vector<std::vector<AToken>> altATokenVec = getAlternativeATokenVector(tokens);
    if (altATokenVec.empty()) {
        return {};
    }
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

auto Tokenizer::splitCandidates(std::span<std::vector<AToken>> candidates) -> CandidateSplit
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
            alt.candidates.push_back(tokenVec);
        }
        alt.key += candidate.front().str.front();
        maxLen--;
        if (maxLen == 0) {
            break;
        }
    }

    return alt;
}

auto Tokenizer::findEndItForLengthOfAlternativeSplit(std::vector<Token>::const_iterator firstSplit,
                                                     const CandidateSplit& candidateSplit)
        -> std::vector<Token>::const_iterator
{
    auto lastSplit = firstSplit;
    std::size_t length = 0;
    while (length < candidateSplit.key.length()) {
        // spdlog::info("csk: {}", candidateSplit.key.string());
        lastSplit = std::next(lastSplit);
        length = std::accumulate(firstSplit, lastSplit, std::size_t{},
                                 [](std::size_t len, const Token& token) -> std::size_t {
                                     // spdlog::info("t: {}", token.string());
                                     return token.getValue().length() + len;
                                 });
    }
    // spdlog::info("length: {}, csl: {}", length, candidateSplit.key.length());
    assert(length == candidateSplit.key.length());
    return lastSplit;
}

auto Tokenizer::getAlternatives(const std::string& text, const std::vector<Token>& currentSplit)
        -> std::vector<Alternative>
{
    std::vector<Alternative> alternatives;
    auto candidates = getCandidates({text}, *wordDB->getDictionary());
    auto firstCandidate = candidates.begin();
    auto firstSplit = currentSplit.begin();
    auto span = std::span(firstCandidate, candidates.end());
    while (!span.empty()) {
        auto alternative = Alternative{};
        auto alt = splitCandidates(span);
        std::advance(firstCandidate, alt.key.length());
        span = std::span(firstCandidate, candidates.end());

        auto lastSplit = findEndItForLengthOfAlternativeSplit(firstSplit, alt);
        ranges::transform(firstSplit, lastSplit, std::back_inserter(alternative.current), &Token::getValue);
        firstSplit = lastSplit;

        auto altATokenVec = getAlternativeATokenVector(alt.candidates);
        ranges::transform(altATokenVec, std::back_inserter(alternative.candidates),
                          [](const std::vector<AToken>& candidateATokens) -> std::vector<utl::StringU8> {
                              std::vector<utl::StringU8> candidate;
                              ranges::transform(candidateATokens, std::back_inserter(candidate), &AToken::key);
                              return candidate;
                          });
        alternatives.push_back(alternative);
    }
    return alternatives;
}

auto Tokenizer::getSplitForChoices(const TokenizationChoiceVec& choices,
                                   const std::string& text,
                                   const std::vector<Token>& currentSplit)
        -> std::vector<Token>
{
    auto alternatives = getAlternatives(text, currentSplit);
    for (const auto& choice : choices) {
        auto alternativeIt = alternatives.begin();
        while (alternativeIt != alternatives.end()) {
            alternativeIt = ranges::find_if(alternativeIt, alternatives.end(), [&choice](const Alternative& alternative) {
                return utl::concanateStringsU8(alternative.current) == utl::concanateStringsU8(choice);
            });
            if (alternativeIt != alternatives.end()) {
                alternativeIt->current = choice;
                std::advance(alternativeIt, 1);
            }
        }
    }
    std::vector<Token> result;
    for (const auto& alternative : alternatives) {
        for (const auto& str : alternative.current) {
            if (auto word = wordDB->lookup(str)) {
                result.emplace_back(str, word);
                continue;
            }
            if (const auto& rule = rules.findRule(str); !rule.empty()) {
                auto word = wordDB->lookup(rule);
                result.emplace_back(str, word);
                continue;
            }
            result.emplace_back(utl::StringU8{str});
        }
    }
    return result;
}

// jieba misses some words, which are present in dictionary - if they contain some elements (like commas).
// joinMissed joins missed words to bigger token.
auto Tokenizer::joinMissed(const std::vector<Token>& splitVector, const std::string& text)
        -> std::vector<Token>
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
        if (auto alt = splitCandidates(span); !alt.candidates.empty()) {
            altATokenVec = getAlternativeATokenVector(alt.candidates);
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
                                  auto word = wordDB->lookup(token.str);
                                  return {token.str, word};
                              });
            continue;
        }
        result.emplace_back(utl::StringU8{str});
    }
    result = joinMissed(result, text);

    ranges::copy(splitVector, std::inserter(allWords, allWords.begin()));

    return result;
}

auto Tokenizer::splitFurther(const std::string& text) -> std::vector<AToken>
{
    std::vector<std::string> splitVector = jieba.cutAll(text);
    if (splitVector.size() == 1) {
        return {};
    }
    if (ranges::none_of(splitVector, [](const std::string& tk) -> bool {
            auto strU8 = utl::StringU8{tk};
            return strU8.length() > 1 || strU8.front().string().length() > 1;
        })) {
        std::vector<AToken> result;
        ranges::transform(splitVector, std::back_inserter(result),
                          [](const auto& str) -> AToken {
                              return {.key = {}, .str = str};
                          });
        return result;
    }
    if (splitVector.size() == 1) {
        auto frontSplitU8 = utl::StringU8(splitVector.front());
        if (frontSplitU8.length() == 1) {
            return {};
        }
        std::vector<AToken> result;
        std::transform(frontSplitU8.cbegin(), frontSplitU8.cend(), std::back_inserter(result),
                       [](const utl::CharU8 charU8) -> AToken {
                           return {.key = {}, .str = charU8.string()};
                       });
        return result;
    }
    auto candidates = getCandidates(text, *wordDB->getDictionary());

    return chooseCombination(candidates);
}

} // namespace annotation
