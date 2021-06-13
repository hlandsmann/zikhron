#include "ZH_Annotator.h"
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <unicode/unistr.h>
#include <utils/StringU8.h>
#include <algorithm>
#include <iostream>
#include <limits>
#include <numeric>
#include <ranges>
#include <span>
#include <string_view>

using std::cout;
namespace ranges = std::ranges;
namespace {

auto GetCandidates(const utl::StringU8& text,
                   const std::span<const ZH_Dictionary::Key>& characterSet,
                   const ZH_Dictionary& dict) -> std::vector<std::vector<ZH_Annotator::ZH_dicItemVec>> {
    std::vector<std::vector<ZH_Annotator::ZH_dicItemVec>> candidates;
    candidates.reserve(text.length());
    for (size_t indexBegin = 0; indexBegin < text.length(); indexBegin++) {
        const auto span_lower = ZH_Dictionary::Lower_bound(text.substr(indexBegin, 1), characterSet);
        const auto span_now = ZH_Dictionary::Upper_bound(text.substr(indexBegin, 1), span_lower);

        std::vector<ZH_Annotator::ZH_dicItemVec> Items;
        for (size_t indexEnd = indexBegin + 1; indexEnd <= text.length(); indexEnd++) {
            const auto key = text.substr(indexBegin, indexEnd - indexBegin);
            const auto found = ZH_Dictionary::Lower_bound(key, span_now);

            if (found.empty() || found.begin()->key.substr(0, key.length()) != key)
                break;
            ZH_Annotator::ZH_dicItemVec dicEntries;
            for (ZH_Dictionary::Key dictionaryKey : found) {
                if (dictionaryKey.key == key) {
                    dicEntries.push_back(dict.ItemFromPosition(dictionaryKey.pos, characterSet));
                } else
                    break;
            }
            if (!dicEntries.empty())
                Items.push_back(std::move(dicEntries));
        }
        candidates.push_back(std::move(Items));
    }

    return candidates;
}

auto GetChunks(const std::vector<std::vector<ZH_Annotator::ZH_dicItemVec>>& candidates,
               const std::span<const ZH_Dictionary::Key>& keys,
               const ZH_Dictionary& dict) -> std::vector<std::vector<std::vector<int>>> {
    using namespace ranges;

    using std::vector;
    vector<vector<int>> chunk;
    vector<vector<vector<int>>> chunks;

    const auto characterSet = dict.CharacterSetFromKeySpan(keys);

    auto candidateLength = [&dict, &characterSet](const ZH_Annotator::ZH_dicItemVec& itemVec) -> int {
        return utl::StringU8(itemVec.front().key).length();
    };
    auto c_lengths = candidates |
                     views::transform([&](auto& v) { return v | views::transform(candidateLength); });

    int forward = 0;
    for (const auto& c_length : c_lengths) {
        std::vector<int> v;
        ranges::copy(c_length, std::back_inserter(v));
        forward = std::max(forward - 1, v.empty() ? 0 : *ranges::max_element(v));
        chunk.push_back(std::move(v));
        if (forward <= 1)
            chunks.push_back(std::move(chunk));
    }

    return chunks;
}

auto get_combinations(const std::vector<std::vector<int>>& chunk) -> std::vector<std::vector<int>> {
    using std::vector;
    vector<vector<int>> combis;
    vector<int> comb;
    size_t pos = 0;

    auto fill_foward = [&pos, &comb, &chunk]() {
        while (pos < chunk.size()) {
            const auto& node = chunk[pos];
            if (node.empty())
                break;
            pos += node.back();
            comb.push_back(node.back());
        }
    };
    auto next_combination = [&pos, &comb, &chunk]() {
        namespace ranges = std::ranges;
        while (not comb.empty()) {
            const int forward = comb.back();
            comb.pop_back();
            pos -= forward;

            const auto& node = chunk[pos];
            const auto it = ranges::find(node, forward);

            if (it == node.begin())
                continue;
            pos += *std::prev(it);
            comb.push_back(*std::prev(it));
            break;
        }
    };
    while (true) {
        fill_foward();
        if (pos == chunk.size())
            combis.push_back(comb);
        next_combination();

        if (comb.empty())
            break;
    }
    return combis;
}

auto compare_combination(const std::vector<int>& a, const std::vector<int>& b) -> bool {
    auto meanDiff = [](const std::vector<int>& vec) {
        if (vec.empty())
            return std::numeric_limits<int>::max();
        const int max = *std::max_element(vec.begin(), vec.end());

        return std::accumulate(
            vec.begin(), vec.end(), int(0), [max](int sum, int val) { return sum + (max - val); });
    };
    if (a.size() == b.size())
        return meanDiff(a) < meanDiff(b);
    return a.size() < b.size();
}
}  // namespace

auto ZH_Annotator::Item::operator<=>(const Item& other) const -> std::weak_ordering {
    if (auto cmp = text <=> other.text; cmp != 0)
        return cmp;
    return dicItemVec <=> other.dicItemVec;
}

ZH_Annotator::ZH_Annotator(const utl::StringU8& _text, const std::shared_ptr<ZH_Dictionary>& _dictionary)
    : text(_text), dictionary(_dictionary) {
    annotate();
}

auto ZH_Annotator::Annotated() const -> const std::string& { return annotated_text; }

auto ZH_Annotator::Items() const -> const std::vector<Item>& { return items; }

auto ZH_Annotator::UniqueItems() const -> std::set<Item> {
    std::set<Item> uniqueItems;

    std::copy_if(items.begin(),
                 items.end(),
                 std::inserter(uniqueItems, uniqueItems.begin()),
                 [](const Item& item) { return not item.dicItemVec.empty(); });

    return uniqueItems;
}

auto ZH_Annotator::Candidates() const -> const std::vector<std::vector<ZH_dicItemVec>>& {
    return candidates;
}
auto ZH_Annotator::Chunks() const -> const std::vector<std::vector<std::vector<int>>>& { return chunks; }

void ZH_Annotator::annotate() {
    using utl::StringU8;
    namespace ranges = std::ranges;

    const auto keys = dictionary->Simplified();
    candidates = GetCandidates(text, keys, *dictionary);
    chunks = GetChunks(candidates, keys, *dictionary);

    namespace ranges = std::ranges;
    namespace views = std::ranges::views;
    const auto& min_combis = chunks | views::transform([](const auto& chunk) -> std::vector<int> {
                                 const auto combs = get_combinations(chunk);
                                 if (not combs.empty())
                                     return *std::min_element(
                                         combs.begin(), combs.end(), compare_combination);
                                 else
                                     return {};
                             });
    int pos = 0;
    for (const auto& comb : min_combis) {
        if (comb.empty()) {
            items.push_back(text.at(pos));
            pos++;
            continue;
        }
        ranges::transform(comb, std::back_inserter(items), [&](const auto& length) {
            const auto itemIt = ranges::find(candidates[pos], size_t(length), [](const auto& itemVec) {
                return StringU8(itemVec.front().key).length();
            });

            pos += length;
            return Item((*itemIt).front().key, std::move(*itemIt));
        });
    }
}
