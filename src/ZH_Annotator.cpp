#include "ZH_Annotator.h"
#include <unicode/unistr.h>
#include <utils/StringU8.h>
#include <algorithm>
#include <iostream>
#include <limits>
#include <numeric>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/transform.hpp>
#include <ranges>
#include <span>
#include <string_view>

using std::cout;

namespace {

auto GetCandidates(const utl::StringU8& text,
                   const std::span<const ZH_Dictionary::Key>& characterSet,
                   const ZH_Dictionary& dict) -> std::vector<std::vector<ZH_Dictionary::Item>> {
    std::vector<std::vector<ZH_Dictionary::Item>> candidates;
    candidates.reserve(text.length());

    for (size_t indexBegin = 0; indexBegin < text.length(); indexBegin++) {
        const auto span_lower = ZH_Dictionary::Lower_bound(text.substr(indexBegin, 1), characterSet);
        const auto span = ZH_Dictionary::Upper_bound(text.substr(indexBegin, 1), span_lower);
        std::vector<ZH_Dictionary::Item> Items;
        for (size_t indexEnd = indexBegin; indexEnd < text.length(); indexEnd++) {
            const auto key = text.substr(indexBegin, indexEnd - indexBegin);
            const auto found = ZH_Dictionary::Lower_bound(key, span);
            if (found.empty() || found.begin()->key.substr(0, key.length()) != key)
                break;
            if (found.front().key == key)
                Items.push_back(dict.ItemFromPosition(found.front().pos, characterSet));
        }
        candidates.push_back(std::move(Items));
    }

    return candidates;
}

auto GetChunks(const std::vector<std::vector<ZH_Dictionary::Item>>& candidates,
               const std::span<const ZH_Dictionary::Key>& keys,
               const ZH_Dictionary& dict) -> std::vector<std::vector<std::vector<int>>> {
    using namespace ranges;
    // namespace ranges = std::ranges;
    // namespace views = std::ranges::views;
    using std::vector;
    vector<vector<int>> chunk;
    vector<vector<vector<int>>> chunks;

    const auto characterSet = dict.CharacterSetFromKeySpan(keys);

    auto candidateLength = [&dict, &characterSet](const ZH_Dictionary::Item& item) -> int {
        return utl::StringU8(item.key).length();
    };
    auto c_lengths = candidates |
                     views::transform([&](auto& v) { return v | views::transform(candidateLength); });

    int forward = 0;
    for (const auto& c_length : c_lengths) {
        std::vector v = c_length | ranges::to<std::vector>();
        forward = std::max(forward - 1, v.empty() ? 0 : *std::max_element(v.begin(), v.end()));
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
            vec.begin(), vec.end(), 0, [max](int sum, int val) { return sum + (max - val); });
    };
    if (a.size() == b.size())
        return meanDiff(a) < meanDiff(b);
    return a.size() < b.size();
}
}  // namespace

void ZH_Annotator::annotate() {
    cout << text << "\n";
    using utl::StringU8;

    const auto keys = dictionary->Simplified();
    auto candidates = GetCandidates(text, keys, *dictionary);
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
    cout << "\n";
    std::string result;
    result += "<span style=\"color:#fff;\">";
    result += "</span>";
    result += "&#8288;";

    int pos = 0;

    for (const auto& comb : min_combis) {
        if (comb.empty()) {
            items.push_back(text.at(pos));
            pos++;
            continue;
        }
        for (const size_t length : comb) {
            const auto& dicItems = candidates[pos];
            pos += length;

            const auto itemIt = ranges::find_if(
                dicItems, [length](const auto& item) { return length == StringU8(item.key).length(); });
            // cout << (*itemIt).key << " ";
            items.push_back((*itemIt).key);
        }
    }
    cout << "<";
    for (const auto& str : items)
        cout << str;
    cout << ">\n";
}
