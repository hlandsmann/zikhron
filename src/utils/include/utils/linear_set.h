#pragma once
#include <spdlog/spdlog.h>
#include <algorithm>
#include <concepts>
#include <limits>
#include <vector>
namespace utl {
template <std::three_way_comparable value_t> class linear_set {
    const size_t sort_at_length;
    bool sorted = true;
    std::vector<value_t> data;
    size_t sorted_until = 0;

public:
    using value_type = value_t;
    using const_iterator = decltype(data)::const_iterator;

    // linear_set(size_t _sort_at_length = std::numeric_limits<size_t>::max())
    //     : sort_at_length(_sort_at_length){};
    linear_set(size_t _sort_at_length = 12) : sort_at_length(_sort_at_length){};

    bool contains(const value_t& v) { return find(v) != data.cend(); }
    auto find(const value_t& v) { return find(data.cbegin(), v); }
    auto find(const_iterator first, const value_t& v) {
        auto last_sorted = data.cbegin() + sorted_until;
        const_iterator it = last_sorted;
        if (first < last_sorted) {
            it = find_sorted(first, v);
            first = last_sorted;
        }
        if (it != last_sorted)
            return it;
        else
            // return std::find(first, data.cend(), v);
            return std::find(data.cbegin(), data.cend(), v);
    }
    auto find_sorted(const_iterator first, const value_t& v) {
        auto last = data.cbegin() + sorted_until;
        auto it = std::lower_bound(first, last, v);
        if (it != last && *it == v) {
            return it;
        }
        assert(std::find(first, last, v) == last);
        return last;
    }

    auto insert(const_iterator, const value_t& v) {
        if (data.empty()) {
            data.push_back(v);
            sorted_until = 1;
            return data.cbegin();
        }
        if (sorted) {
            assert(data.size() == sorted_until);
            spdlog::info("sorted v {}, u: {}", v, sorted_until);
            if (v == data.back())
                return std::prev(data.cend());
            if (v < data.back()) {
                spdlog::info("false v: {}", v);

                sorted_until = data.size();
                if (auto it = std::find(data.cbegin(), data.cend(), v); it != data.cend()) {
                    spdlog::error(
                        "v: {}, d: {}, su: {}", v, std::distance(data.cbegin(), it), sorted_until);
                    spdlog::error("vec: [{}]", fmt::join(data, ", "));
                    auto last = data.cbegin() + sorted_until;
                    auto it2 = std::lower_bound(data.cbegin(), last, v);
                    spdlog::error("it_d: {}, v: {}, *it: {}", std::distance(data.cbegin(), it2), v, *it2);
                }
                data.push_back(v);
                sorted = false;
            } else {
                assert(std::find(data.cbegin(), data.cend(), v) == data.cend());
                data.push_back(v);
                sorted_until = data.size();
                spdlog::warn("true v: {}, u: {}", v, sorted_until);
            }

            return std::prev(data.cend());
        }

        if (auto it = find(v); it != data.cend())
            return it;
        assert(std::find(data.cbegin(), data.cend(), v) == data.cend());
        data.push_back(v);

        if (sort_at_length < data.size() - sorted_until)
            sort();
        return std::prev(data.cend());
    }

    auto begin() const { return data.begin(); }
    auto end() const { return data.end(); }
    auto size() const { return data.size(); }

    size_t times_sorted = 0;
    void sort() {
        std::sort(data.begin() + sorted_until, data.end());
        std::inplace_merge(data.begin(), data.begin() + sorted_until, data.end());
        sorted_until = data.size();
        spdlog::critical("sort: {}", sorted_until);

        sorted = true;
        times_sorted++;
    }
    auto is_sorted() { return std::is_sorted(data.begin(), data.end()); }

private:
};
}  // namespace utl
