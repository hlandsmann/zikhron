#include <VocabularySR.h>
#include <fmt/format.h>
#include <utils/min_element_val.h>
#include <algorithm>
#include <chrono>
#include <functional>
#include <limits>
#include <ranges>
namespace ranges = std::ranges;

auto time_it(std::function<void()> func) {
    namespace chrono = std::chrono;
    auto start = chrono::high_resolution_clock::now();
    func();
    auto end = chrono::high_resolution_clock::now();
    return chrono::duration_cast<chrono::milliseconds>(end - start).count();
}

VocabularySR_X::VocabularySR_X(const std::map<uint, VocableSR>& _id_vocableSR,
                               const std::map<uint, CardMeta>& _id_cardMeta,
                               const std::map<uint, VocableMeta>& _id_vocableMeta)
    : id_vocableSR(_id_vocableSR), id_cardMeta(_id_cardMeta), id_vocableMeta(_id_vocableMeta) {
    worker = std::jthread([this](std::stop_token token) {
        // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        fmt::print("Thread start <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
        auto init = time_it([this]() { initDataStructure(); });
        fmt::print("Init: {}ms\n", init);
        fmt::print("Thread end   <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
    });
}
void VocabularySR_X::initDataStructure() {
    ranges::copy(id_vocableSR, std::back_inserter(repeatTodayVoc));

    // for (const auto& [id, vocSR] : id_vocableSR) {
    //     if (vocSR.isToBeRepeatedToday())
    //         ids_repeatTodayVoc.push_back(id);
    //     // if (vocSR.isAgainVocable())
    //     //     ids_againVoc.insert(id);
    // }
}
