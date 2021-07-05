#include <fmt/format.h>
#include <algorithm>
#include <chrono>
#include <cppcoro/generator.hpp>
#include <ranges>

#include "VocabularySR.h"
namespace ranges = std::ranges;

template <class T> auto PopFront(std::set<T>& set) -> cppcoro::generator<T> {
    while (not set.empty()) {
        auto nextIt = set.begin();
        T next = *nextIt;
        set.erase(nextIt);
        co_yield next;
    }
}

template <class T> auto EachOnce(std::set<T>& set) -> cppcoro::generator<T> {
    std::set<T> seenElements;
    while (seenElements != set) {
        for (T element : set) {
            if (not seenElements.contains(element)) {
                co_yield element;
                seenElements.insert(element);
            }
        }
    }
}

auto VocabluarySR_TreeWalker::SplitGroup(const Group& group) -> std::vector<Group> {
    std::vector<Group> result;
    std::set<uint> vocIdsInput;
    ranges::copy(group.id_vocMeta | std::views::keys, std::inserter(vocIdsInput, vocIdsInput.begin()));

    for (uint vocId : PopFront(vocIdsInput)) {
        std::set<uint> groupedVocIds = {vocId};
        std::map<uint, CardMeta> id_cm;
        std::map<uint, VocableMeta> id_vocMeta;

        auto cardIds = group.id_vocMeta.at(vocId).cardIds;

        for (uint cardId : EachOnce(cardIds)) {
            auto newVoc = group.id_cardMeta.at(cardId).vocableIds |
                          std::views::filter([&](uint id) { return vocIdsInput.contains(id); });
            for (uint newVocId : newVoc) {
                vocIdsInput.erase(newVocId);
                groupedVocIds.insert(newVocId);
                ranges::copy(group.id_vocMeta.at(newVocId).cardIds,
                             std::inserter(cardIds, cardIds.begin()));
            }

            ranges::transform(
                cardIds, std::inserter(id_cm, id_cm.begin()), [&](uint id) -> std::pair<uint, CardMeta> {
                    CardMeta cm;
                    ranges::copy_if(
                        group.id_cardMeta.at(id).vocableIds,
                        std::inserter(cm.vocableIds, cm.vocableIds.begin()),
                        [&](uint containedVocId) { return group.id_vocMeta.contains(containedVocId); });
                    return {id, cm};  // id_cardMeta.at(id)};
                });
        }
        ranges::transform(groupedVocIds,
                          std::inserter(id_vocMeta, id_vocMeta.begin()),
                          [&](uint vid) -> std::pair<uint, VocableMeta> {
                              return {vid, group.id_vocMeta.at(vid)};
                          });
        result.push_back({.id_vocMeta = std::move(id_vocMeta), .id_cardMeta = std::move(id_cm)});
    }
    return result;
}

VocabluarySR_TreeWalker::VocabluarySR_TreeWalker(const std::map<uint, VocableSR>& _id_vocableSR,
                                                 const std::map<uint, CardMeta>& _id_cardMeta,
                                                 const std::map<uint, VocableMeta>& _id_vocableMeta)
    : id_vocableSR(_id_vocableSR), id_cardMeta(_id_cardMeta), id_vocableMeta(_id_vocableMeta) {
    worker = std::jthread([this](std::stop_token token) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        fmt::print("Thread start <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
        namespace chrono = std::chrono;
        using microseconds = std::chrono::milliseconds;
        auto start = chrono::high_resolution_clock::now();
        while (not token.stop_requested()) {
            // fmt::print("Print from thread\n");
            break;
        }
        std::set<uint> ids_repeatTodayVoc;
        std::set<uint> ids_againVoc;
        for (const auto& [id, vocSR] : id_vocableSR) {
            if (vocSR.isToBeRepeatedToday())
                ids_repeatTodayVoc.insert(id);
            if (vocSR.isAgainVocable())
                ids_againVoc.insert(id);
        }
        std::map<uint, CardMeta> group_id_cardMeta;
        for (const auto& [id, cardMeta] : id_cardMeta) {
            std::set<uint> vocableIds;
            ranges::copy_if(
                cardMeta.vocableIds,
                std::inserter(vocableIds, vocableIds.begin()),
                [ids_repeatTodayVoc](uint vocId) { return ids_repeatTodayVoc.contains(vocId); });
            if (not vocableIds.empty())
                group_id_cardMeta[id] = {.vocableIds = std::move(vocableIds)};
        }
        std::map<uint, VocableMeta> group_id_vocMeta;
        ranges::transform(ids_repeatTodayVoc,
                          std::inserter(group_id_vocMeta, group_id_vocMeta.begin()),
                          [this](uint vocId) -> std::pair<uint, VocableMeta> {
                              return {vocId, id_vocableMeta.at(vocId)};
                          });
        Group startGroup = {.id_vocMeta = group_id_vocMeta, .id_cardMeta = group_id_cardMeta};

        fmt::print("IdCardMeta size: {}\n", startGroup.id_cardMeta.size());
        std::vector<Group> groups = SplitGroup(startGroup);

        fmt::print("Found {} trees\n", groups.size());
        int countVoc = 0;
        for (const auto& group : groups) {
            countVoc += group.id_vocMeta.size();
        }
        fmt::print("Vocables to study total: {}\n", countVoc);
        fmt::print("--------------------------------------------\n");

        ranges::sort(
            groups, ranges::greater{}, [](const Group& group) { return group.id_vocMeta.size(); });
        for (const Group& group : groups | std::views::take(13)) {
            fmt::print(
                "Group voc size {}, card size {}\n", group.id_vocMeta.size(), group.id_cardMeta.size());
        }
        fmt::print("--------------------------------------------\n");

        ProcessGroup(groups.front());

        auto thr = chrono::high_resolution_clock::now();
        fmt::print("Time for thread: {} \n", chrono::duration_cast<microseconds>(thr - start).count());

        fmt::print("Thread end >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
    });
}

void VocabluarySR_TreeWalker::ProcessGroup(Group& group) {
    std::map<uint, uint> vocId_count;

    std::vector<std::pair<uint, CardMeta>> v_id_cardMeta;
    ranges::copy(group.id_cardMeta, std::back_inserter(v_id_cardMeta));
    ranges::sort(v_id_cardMeta, ranges::greater{}, [](const auto& id_cm) {
        return id_cm.second.vocableIds.size();
    });

    for (const auto& [id, cn] : group.id_cardMeta) {
        for (uint vocId : cn.vocableIds)
            vocId_count[vocId]++;
    }
    std::vector<std::pair<uint, uint>> vocId_count_vec;
    ranges::copy(vocId_count, std::back_inserter(vocId_count_vec));
    ranges::sort(vocId_count_vec, std::greater{}, &std::pair<uint, uint>::second);

    for (auto [vocId, count] : vocId_count_vec | std::views::take(10)) {
        fmt::print("most often: id {}, count {}\n", vocId, count);
    }
    fmt::print("--------------------------------------------\n");
    for (auto [cid, cm] : v_id_cardMeta | std::views::take(10)) {
        fmt::print("card_id: id {}, voc_count {}\n", cid,cm.vocableIds.size() );
    }
    fmt::print("--------------------------------------------\n");}
