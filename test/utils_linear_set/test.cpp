#include <spdlog/spdlog.h>
#include <utils/linear_set.h>
#include <algorithm>
#include <catch2/catch.hpp>
#include <random>
#include <ranges>
#include <set>

namespace ranges = std::ranges;

static size_t rng_rand(int rng) {
    static std::mt19937 gen32;

    return gen32() % rng;
}

TEST_CASE("linear set") {
    constexpr int rng = 128;
    std::set<unsigned> set;
    utl::linear_set<unsigned> ls;
    std::vector<unsigned> data;
    std::generate_n(std::back_inserter(data), rng, [&]() { return rng_rand(rng); });
    REQUIRE(data.size() == rng);
    std::sort(data.begin(), data.begin() + (rng / 4));

    std::copy(data.begin(), data.end(), std::inserter(set, set.begin()));
    std::copy(data.begin(), data.end(), std::inserter(ls, ls.begin()));
    REQUIRE(set.size() == ls.size());
    REQUIRE(ranges::all_of(std::ranges::iota_view{0, rng}, [&](unsigned val) {
        // spdlog::info("val: {}, contains: {}", val, ls.contains(val));
        return set.contains(val) == ls.contains(val);
    }));

    ls.sort();
    REQUIRE(ls.is_sorted());
    REQUIRE(set.size() == ls.size());
    REQUIRE(ranges::all_of(std::ranges::iota_view{0, rng}, [&](unsigned val) {
        // spdlog::info("val: {}, contains: {}", val, ls.contains(val));
        return set.contains(val) == ls.contains(val);
    }));
}
