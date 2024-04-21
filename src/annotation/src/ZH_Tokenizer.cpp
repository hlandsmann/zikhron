#include "ZH_Tokenizer.h"

#include "dictionary/ZH_Dictionary.h"

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <unicode/unistr.h>
#include <utils/StringU8.h>

#include <algorithm>
#include <compare>
#include <cstddef>
#include <gsl/util>
#include <iterator>
#include <limits>
#include <memory>
#include <numeric>
#include <ranges>
#include <set>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace ranges = std::ranges;
namespace {

auto GetCandidates(const utl::StringU8& text,
                   const CharacterSetType characterSet,
                   const ZH_Dictionary& dict) -> std::vector<std::vector<ZH_Tokenizer::ZH_dicItemVec>>
{
    std::vector<std::vector<ZH_Tokenizer::ZH_dicItemVec>> candidates;
    candidates.reserve(text.length());
    for (int indexBegin = 0; indexBegin < static_cast<int>(text.length()); indexBegin++) {
        const auto span_lower = ZH_Dictionary::Lower_bound(text.substr(indexBegin, 1), dict.Simplified());
        const auto span_now = ZH_Dictionary::Upper_bound(text.substr(indexBegin, 1), span_lower);

        std::vector<ZH_Tokenizer::ZH_dicItemVec> Items;
        for (int indexEnd = indexBegin + 1; indexEnd <= static_cast<int>(text.length()); indexEnd++) {
            const auto key = text.substr(indexBegin, indexEnd - indexBegin);
            const auto found = ZH_Dictionary::Lower_bound(key, span_now);

            if (found.empty() || found.begin()->key.substr(0, key.length()) != key) {
                break;
            }
            ZH_Tokenizer::ZH_dicItemVec dicEntries;
            for (ZH_Dictionary::Key dictionaryKey : found) {
                if (dictionaryKey.key == key) {
                    dicEntries.push_back(dict.entryFromPosition(dictionaryKey.pos, characterSet));
                } else {
                    break;
                }
            }
            if (!dicEntries.empty()) {
                Items.push_back(std::move(dicEntries));
            }
        }
        candidates.push_back(std::move(Items));
    }

    return candidates;
}

auto GetChunks(const std::vector<std::vector<ZH_Tokenizer::ZH_dicItemVec>>& candidates)
        -> std::vector<std::vector<std::vector<int>>>
{
    namespace views = ranges::views;

    using std::vector;
    vector<vector<int>> chunk;
    vector<vector<vector<int>>> chunks;

    auto candidateLength = [](const ZH_Tokenizer::ZH_dicItemVec& itemVec) -> size_t {
        return utl::StringU8(itemVec.front().key).length();
    };
    auto c_lengths = candidates | views::transform([&](auto& v) { return v | views::transform(candidateLength); });

    int forward = 0;
    for (const auto& c_length : c_lengths) {
        std::vector<int> v;
        ranges::copy(c_length, std::back_inserter(v));
        forward = std::max(forward - 1, v.empty() ? 0 : *ranges::max_element(v));
        chunk.push_back(std::move(v));
        if (forward <= 1) {
            chunks.push_back(std::move(chunk));
            chunk.clear();
        }
    }

    return chunks;
}

auto compare_combination(const std::vector<int>& a, const std::vector<int>& b) -> bool
{
    auto meanDiff = [](const std::vector<int>& vec) {
        if (vec.empty()) {
            return std::numeric_limits<int>::max();
        }
        const int max = *std::max_element(vec.begin(), vec.end());

        return std::accumulate(
                vec.begin(), vec.end(), int(0), [max](int sum, int val) { return sum + (max - val); });
    };
    if (a.size() == b.size()) {
        return meanDiff(a) < meanDiff(b);
    }
    return a.size() < b.size();
}
} // namespace

auto ZH_Tokenizer::get_combinations(const std::vector<Combination>& chunk) -> std::vector<Combination>
{
    using std::vector;
    vector<Combination> combis;
    Combination comb;
    size_t pos = 0;

    auto fill_foward = [&pos, &comb, &chunk]() {
        while (pos < chunk.size()) {
            const auto& node = chunk[pos];
            if (node.empty()) {
                break;
            }
            pos += static_cast<size_t>(node.back());
            comb.push_back(node.back());
        }
    };
    auto next_combination = [&pos, &comb, &chunk]() {
        namespace ranges = std::ranges;
        while (not comb.empty()) {
            auto forward = static_cast<size_t>(comb.back());
            comb.pop_back();
            pos -= forward;

            const auto& node = chunk[pos];
            const auto it = ranges::find(node, forward);

            if (it == node.begin()) {
                continue;
            }
            pos += static_cast<size_t>(*std::prev(it));
            comb.push_back(*std::prev(it));
            break;
        }
    };
    while (true) {
        fill_foward();
        if (pos == chunk.size()) {
            combis.push_back(comb);
        }
        next_combination();

        if (comb.empty()) {
            break;
        }
    }
    return combis;
}

auto ZH_Tokenizer::Token::operator<=>(const Token& other) const -> std::weak_ordering
{
    if (auto cmp = text <=> other.text; cmp != nullptr) {
        return cmp;
    }
    return dicItemVec <=> other.dicItemVec;
}

ZH_Tokenizer::ZH_Tokenizer(utl::StringU8 _text,
                           std::shared_ptr<const ZH_Dictionary> _dictionary,
                           std::shared_ptr<const AnnotationChoiceMap> _choices)
    : text{std::move(_text)}
    , dictionary{std::move(_dictionary)}
    , choices{std::move(_choices)}
{
    annotate();
}

auto ZH_Tokenizer::Annotated() const -> const std::string&
{
    return annotated_text;
}

auto ZH_Tokenizer::Tokens() const -> const std::vector<Token>&
{
    return tokens;
}

auto ZH_Tokenizer::UniqueItems() const -> std::set<Token>
{
    std::set<Token> uniqueItems;

    std::copy_if(tokens.begin(),
                 tokens.end(),
                 std::inserter(uniqueItems, uniqueItems.begin()),
                 [](const Token& item) { return not item.dicItemVec.empty(); });

    return uniqueItems;
}

auto ZH_Tokenizer::Candidates() const -> const std::vector<std::vector<ZH_dicItemVec>>&
{
    return candidates;
}

auto ZH_Tokenizer::Chunks() const -> const std::vector<std::vector<std::vector<int>>>&
{
    return chunks;
}

auto ZH_Tokenizer::Dictionary() const -> const std::shared_ptr<const ZH_Dictionary>&
{
    return dictionary;
}

void ZH_Tokenizer::annotate()
{
    using utl::StringU8;

    candidates = GetCandidates(text, CharacterSetType::Simplified, *dictionary);
    chunks = GetChunks(candidates);

    namespace ranges = std::ranges;
    namespace views = std::ranges::views;
    auto min_combis =
            chunks | views::transform([this, pos = 0](const auto& chunk) mutable -> std::vector<int> {
                const auto combs = get_combinations(chunk);
                if (combs.empty()) {
                    pos++;
                    return {};
                }

                int combinationLength = std::accumulate(combs.front().begin(), combs.front().end(), 0);
                auto finally = gsl::final_action([&pos, combinationLength]() { pos += combinationLength; });
                if (combinationLength > 1) {
                    const auto& possibleChoice = std::vector<utl::CharU8>(
                            text.cbegin() + pos, text.cbegin() + pos + combinationLength);
                    const auto& choiceIt = choices->find(possibleChoice);
                    if (choiceIt != choices->end()) {
                        return choiceIt->second;
                    }
                }
                return *std::min_element(combs.begin(), combs.end(), compare_combination);
            });
    size_t pos = 0;
    for (const auto& comb : min_combis) {
        if (comb.empty()) {
            tokens.emplace_back(text.at(pos));
            pos++;
            continue;
        }

        ranges::transform(comb, std::back_inserter(tokens), [&](const size_t length) {
            const auto itemIt = ranges::find(candidates[pos], size_t(length), [](const auto& itemVec) {
                return StringU8(itemVec.front().key).length();
            });

            pos += length;
            return Token((*itemIt).front().key, std::move(*itemIt));
        });
    }
}

// void ZH_Tokenizer::SetAnnotationChoices(const std::map<CharacterSequence, Combination>& _choices)
// {
//     choices = _choices;
// }

void ZH_Tokenizer::Reannotate()
{
    tokens.clear();
    annotate();
}

auto ZH_Tokenizer::ContainsCharacterSequence(const CharacterSequence& charSeq) -> bool
{
    return not ranges::search(std::span(text.cbegin(), text.cend()), charSeq).empty();
}
