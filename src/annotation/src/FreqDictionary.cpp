#include "FreqDictionary.h"

#include <utils/format.h>
#include <utils/string_split.h>

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace ranges = std::ranges;

namespace annotation {

FreqDictionary::FreqDictionary()
{
    loadFreq(dict_in_path);
    loadIdf(idf_in_path);
}

template<class Frequency>
auto FreqDictionary::find(std::string_view key) const
        -> std::optional<Frequency const*>
{
    // A reference to std::vector<Frequency> would be correct, but the compiler generates a warning then.
    // Todo change back with newer compiler version.
    const auto* container = [this]() -> const std::vector<Frequency>* {
        if constexpr (std::is_same_v<Frequency, DictFreq>) {
            return &freq;
        } else if constexpr (std::is_same_v<Frequency, IDF>) {
            return &idf;
        }
        std::unreachable();
    }();
    auto span_now = std::span{
            ranges::lower_bound(*container, key, ranges::less{}, &Frequency::key),
            ranges::upper_bound(*container, key, ranges::less{}, &Frequency::key)};
    auto it = ranges::find(span_now, key, &Frequency::key);
    if (it == span_now.end()) {
        return {};
    }
    return {it.base()};
}

auto FreqDictionary::getFreq(std::string_view key) const -> int
{
    auto it = find<DictFreq>(key);
    if (!it.has_value()) {
        return 0;
    }
    return (*it)->freq;
}

auto FreqDictionary::getIdf(std::string_view key) const -> float
{
    auto it = find<IDF>(key);
    if (!it.has_value()) {
        return 0;
    }
    return (*it)->idf;
}

void FreqDictionary::loadFreq(const std::filesystem::path& filename)
{
    freqString = utl::load_string_file(filename);
    auto numberOfEntries = std::count(freqString.begin(), freqString.end(), '\n');
    freq.reserve(static_cast<std::size_t>(numberOfEntries));
    auto rest = std::string_view{freqString};
    while (!rest.empty()) {
        auto key = utl::split_front(rest, ' ');
        auto count = std::string{utl::split_front(rest, ' ')};
        auto tag = utl::split_front(rest, '\n');
        freq.emplace_back(key, std::stoi(count), tag);
        // if (freq.back().freq == 0) {
        //     spdlog::info("{} - {} - {}", key, count, tag);
        // }
    }
    ranges::sort(freq, ranges::less{}, &DictFreq::key);
}

void FreqDictionary::loadIdf(const std::filesystem::path& filename)
{
    idfString = utl::load_string_file(filename);
    auto numberOfEntries = std::count(idfString.begin(), idfString.end(), '\n');
    idf.reserve(static_cast<std::size_t>(numberOfEntries));

    auto rest = std::string_view{idfString};
    while (!rest.empty()) {
        auto key = utl::split_front(rest, ' ');
        auto count = std::string{utl::split_front(rest, '\n')};
        idf.emplace_back(key, std::stof(count));
        // if (idf.back().idf == 0) {
        //     spdlog::info("{} - {}", key, count);
        // }
    }
    ranges::sort(idf, ranges::less{}, &IDF::key);
}

} // namespace annotation
