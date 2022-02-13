#include <benchmark/benchmark.h>
#include <utils/linear_set.h>
#include <array>
#include <random>
#include <set>
#include <vector>

struct DataSet_unique {
    static constexpr int nvectors = 1024;
    DataSet_unique(size_t _length) : length(_length) {
        std::generate(data.begin(), data.end(), [&, this]() -> std::vector<unsigned> {
            std::set<unsigned> newData;
            while (newData.size() != length)
                newData.insert(gen32());
            std::vector<unsigned> v = {newData.begin(), newData.end()};
            std::shuffle(v.begin(), v.end(), gen32);
            return v;
        });
    }
    unsigned nextVal() {
        size_t _index = index++;
        size_t _v = v;
        if (index == length) {
            index = 0;
            v = (v + 1 == data.size()) ? 0 : v + 1;
        }

        return data[_v][_index];
    }

    std::mt19937 gen32;
    size_t index = 0;
    size_t v = 0;
    const size_t length;
    std::array<std::vector<unsigned>, nvectors> data;
};

template <class set>
static void BM_insert(benchmark::State& state) {
    DataSet_unique unique(state.range(0));
    for (auto _ : state) {
        set ls;
        std::generate_n(std::inserter(ls, ls.begin()), state.range(0), [&](){return unique.nextVal();});
    }
}
// BENCHMARK_TEMPLATE(BM_insert, utl::linear_set<unsigned>)->DenseRange(0, 1024, 32);
// BENCHMARK_TEMPLATE(BM_insert, std::set<unsigned>)->DenseRange(0, 1024, 32);

static void BM_sort_at(benchmark::State & state){
    constexpr int data_size = 512;
    DataSet_unique unique(data_size);
    size_t times_sorted = 0;
    for (auto _ : state) {
        utl::linear_set<unsigned> ls(state.range(0));
        std::generate_n(std::inserter(ls, ls.begin()), data_size, [&](){return unique.nextVal();});
        times_sorted = ls.times_sorted;
    }
    // spdlog::info("Times sorted {}", times_sorted);
}
BENCHMARK(BM_sort_at)->DenseRange(0, 256, 2);
BENCHMARK_MAIN();
