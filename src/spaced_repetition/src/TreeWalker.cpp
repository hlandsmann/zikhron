// This is dead code. Just used for reference to build the real treewalker...

#include <fmt/format.h>
#include <utils/min_element_val.h>
#include <algorithm>
#include <chrono>
#include <cppcoro/generator.hpp>
#include <cppcoro/recursive_generator.hpp>
#include <limits>
#include <numeric>
#include <ranges>
#include <utils/counting_iterator.h>

#include "VocabularySR.h"
namespace ranges = std::ranges;

template <class Key, class Compare = std::less<Key>>
auto PopFront(std::set<Key, Compare>& set) -> cppcoro::generator<Key> {
    while (not set.empty()) {
        auto nextIt = set.begin();
        Key next = *nextIt;
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

template <class T> auto AtMostTake(const std::set<T>& set, uint counter) -> std::set<T> {
    std::set<T> result;
    for (auto elem : set) {
        if (counter-- == 0)
            break;
        result.insert(elem);
    }
    return result;
}

auto VocabularySR_TreeWalker::SplitGroup(const Group& group) -> std::vector<Group> {
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

VocabularySR_TreeWalker::VocabularySR_TreeWalker(const std::map<uint, VocableSR>& _id_vocableSR,
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

auto VocabularySR_TreeWalker::OtherCardsWithVocables(const std::map<uint, CardMeta>& id_cm, uint cardId)
    -> std::set<uint> {
    std::set<uint> result;
    const auto& cm = id_cm.at(cardId);
    for (uint vocId : cm.vocableIds) {
        for (uint _cardId : id_vocableMeta.at(vocId).cardIds)
            if (_cardId != cardId && id_cm.contains(_cardId))
                result.insert(_cardId);
    }
    return result;
}

template <std::array quantity> struct preferedQuantity {
    auto operator()(uint a, uint b) const -> bool {
        const auto a_it = ranges::find(quantity, a);
        const auto b_it = ranges::find(quantity, b);
        if (a_it != b_it)
            return a_it < b_it;
        return a < b;
    }
};

auto VocabularySR_TreeWalker::CardsBestSize(const std::map<uint, CardMeta>& id_cm)
    -> std::vector<std::pair<uint, CardMeta>> {
    std::vector<std::pair<uint, CardMeta>> result;
    std::map<uint, uint> length_count;

    std::set<uint> setVocs;

    auto compare_by_urgency = [&id_cm, &setVocs, this](uint a, uint b) {
        auto urgency = [&setVocs, this](uint vocId) {
            if (setVocs.contains(vocId))
                return std::numeric_limits<float>::infinity();
            return id_vocableSR.at(vocId).urgency();
        };
        float a_urgency = utl::min_element_val(
            id_cm.at(a).vocableIds, std::numeric_limits<float>::infinity(), ranges::less{}, urgency);
        float b_urgency = utl::min_element_val(
            id_cm.at(b).vocableIds, std::numeric_limits<float>::infinity(), ranges::less{}, urgency);
        if (a_urgency == b_urgency)
            return a < b;
        return a_urgency < b_urgency;
    };

    std::set<uint> evaluateCardIds;
    using myPreferedQuantity = preferedQuantity<{4, 3, 5, 2, 6}>;
    std::map<uint, std::set<uint, decltype(compare_by_urgency)>, myPreferedQuantity> length_cardIds;

    auto lengthOfCard = [&](uint cardId) {
        return ranges::set_difference(id_cm.at(cardId).vocableIds, setVocs, utl::counting_iterator{})
            .out.count;
    };
    auto removeOtherCardsWithVocables =
        [this, &length_cardIds, &id_cm, &setVocs, &evaluateCardIds, &lengthOfCard](uint cardId) {
            for (uint _cardId : OtherCardsWithVocables(id_cm, cardId)) {
                uint countNewVocs = lengthOfCard(_cardId);
                if (countNewVocs == 0)
                    continue;
                try {
                    if (not evaluateCardIds.contains(_cardId)) {
                        if (not length_cardIds.at(countNewVocs).contains(_cardId)) {
                            bool tf = false;
                            for (const auto& [l, cid] : length_cardIds) {
                                if (cid.contains(_cardId)) {
                                    fmt::print("Expected at: {}, found at: {}\n", countNewVocs, l);
                                    tf = true;
                                }
                            }
                            if (not tf)
                                fmt::print("Missing cid: {}\n", _cardId);
                        }
                    }
                    length_cardIds.at(countNewVocs).erase(_cardId);
                    evaluateCardIds.insert(_cardId);
                } catch (const std::exception& e) {
                    fmt::print("Happened here: {}, what: {}\n", countNewVocs, e.what());
                }
            }
        };

    ranges::copy(id_cm | std::views::keys, std::inserter(evaluateCardIds, evaluateCardIds.begin()));

    while (true) {
        for (const uint cardId : evaluateCardIds) {
            uint countNewVocs = lengthOfCard(cardId);
            if (countNewVocs != 0) {
                if (not length_cardIds.contains(countNewVocs))
                    length_cardIds.emplace(countNewVocs, compare_by_urgency);
                length_cardIds.at(countNewVocs).insert(cardId);
                assert(length_cardIds.at(countNewVocs).contains(cardId));
            }
        }
        evaluateCardIds.clear();

        std::erase_if(length_cardIds, [](const auto& lengthCardIds) {
            const auto& [length, cardIds] = lengthCardIds;
            if (cardIds.empty()) {
                fmt::print("Gonna erase length {}\n", length);
            }
            return cardIds.empty();
        });

        if (length_cardIds.empty())
            break;

        auto& [length, cardIds] = *length_cardIds.begin();
        for (uint cardId : PopFront(cardIds)) {
            removeOtherCardsWithVocables(cardId);
            auto& cardMeta = id_cm.at(cardId);
            ranges::copy(cardMeta.vocableIds, std::inserter(setVocs, setVocs.begin()));
            result.emplace_back(cardId, cardMeta);
            // fmt::print("SetVocSize: {}, length: {}, loc: {}\n",
            //            setVocs.size(),
            //            length,
            //            cardMeta.vocableIds.size());
            length_count[length]++;

            if (ranges::any_of(OtherCardsWithVocables(id_cm, cardId),
                               [&id_cm, &setVocs, &lengthOfCard, length](uint _cardId) {
                                   uint countNewVocs = lengthOfCard(_cardId);
                                   if (countNewVocs == 0)
                                       return false;
                                   return myPreferedQuantity{}(countNewVocs, length);
                               }))
                break;
        }
    }
    fmt::print("SetVocs size: {}\n", setVocs.size());
    for (auto [length, count] : length_count) {
        fmt::print("VocSize: {}, count: {}\n", length, count);
    }

    return result;
}

auto VocabularySR_TreeWalker::RefinedCards(const std::vector<std::pair<uint, CardMeta>>& _id_cm)
    -> std::vector<std::pair<uint, CardMeta>> {
    std::map<uint, uint> length_count;
    std::vector<std::pair<uint, CardMeta>> result;
    std::map<uint, CardMeta> id_cm;
    ranges::copy(_id_cm, std::inserter(id_cm, id_cm.begin()));

    using myPreferedQuantity = preferedQuantity<{4, 3, 2, 1}>;

    std::set<uint> setVocs;
    auto notInSetVocs = [&setVocs](uint vocId) { return not setVocs.contains(vocId); };
    auto lengthOfCard = [&](uint cardId) {
        return ranges::count_if(id_cm.at(cardId).vocableIds, notInSetVocs);
    };
    auto urgency = [this](uint vocId) { return id_vocableSR.at(vocId).urgency(); };
    auto card_minUrgency = [&urgency, &notInSetVocs, &id_cm](uint cardId) {
        return utl::min_element_val(id_cm.at(cardId).vocableIds | std::views::filter(notInSetVocs),
                                    std::numeric_limits<float>::infinity(),
                                    ranges::less{},
                                    urgency);
    };
    auto card_avgUrgency = [&urgency, &notInSetVocs, &id_cm](uint cardId) {
        auto rng = id_cm.at(cardId).vocableIds | std::views::filter(notInSetVocs);
        int counter = 0;
        float sum = std::accumulate(
            ranges::begin(rng), ranges::end(rng), 0., [&counter, &urgency](float a, uint b) {
                counter++;
                return a + urgency(b);
            });
        if (counter == 0)
            return std::numeric_limits<float>::infinity();
        return sum / float(counter);
    };
    auto compare_by_value = [&, this](uint cardIdA, uint cardIdB) {
        if (auto [a, b] = std::pair(lengthOfCard(cardIdA), lengthOfCard(cardIdB)); a != b)
            return myPreferedQuantity{}(a, b);
        if (auto [a, b] = std::pair(card_minUrgency(cardIdA), card_minUrgency(cardIdB)); a != b)
            return a < b;
        if (auto [a, b] = std::pair(card_avgUrgency(cardIdA), card_avgUrgency(cardIdB)); a != b)
            return a < b;
        return cardIdA < cardIdB;
    };

    std::set<uint> evaluateCardIds;
    std::set<uint, decltype(compare_by_value)> cid_wip(compare_by_value);
    ranges::copy(_id_cm | std::views::keys, std::inserter(cid_wip, cid_wip.begin()));

    auto removeOtherCardsWithVocables =
        [this, &cid_wip, &id_cm, &setVocs, &evaluateCardIds, &lengthOfCard](uint cardId) {
            for (uint _cardId : OtherCardsWithVocables(id_cm, cardId)) {
                uint countNewVocs = lengthOfCard(_cardId);
                if (countNewVocs == 0)
                    continue;
                if (not evaluateCardIds.contains(_cardId)) {
                    if (cid_wip.contains(_cardId)) {
                        cid_wip.erase(_cardId);
                        evaluateCardIds.insert(_cardId);
                    }
                }
            }
        };
    while (true) {
        ranges::copy(evaluateCardIds, std::inserter(cid_wip, cid_wip.begin()));
        evaluateCardIds.clear();
        for (uint cardId : PopFront(cid_wip)) {
            if (lengthOfCard(cardId) == 0)
                continue;
            if (ranges::any_of(evaluateCardIds,
                               [&cardId, &compare_by_value, &lengthOfCard](uint _cardId) {
                                   if (lengthOfCard(_cardId) == 0)
                                       return false;
                                   if (lengthOfCard(cardId) > 4)
                                       return compare_by_value(_cardId, cardId);
                                   return false;
                               })) {
                evaluateCardIds.insert(cardId);
                break;
            }

            length_count[lengthOfCard(cardId)]++;

            removeOtherCardsWithVocables(cardId);
            auto& cardMeta = id_cm.at(cardId);
            ranges::copy(cardMeta.vocableIds, std::inserter(setVocs, setVocs.begin()));
            result.emplace_back(cardId, cardMeta);
        }
        if (evaluateCardIds.empty() && cid_wip.empty())
            break;
    }
    fmt::print("SetVocs size: {}\n", setVocs.size());
    int stopAt = 0;
    setVocs.clear();
    for (const auto& [cardId, cardMeta] : result) {
        fmt::print("Length: {}, minUrgency: {}, avgUrgency: {} \n",
                   lengthOfCard(cardId),
                   card_minUrgency(cardId),
                   card_avgUrgency(cardId));
        if (stopAt++ > 10)
            break;
        ranges::copy(cardMeta.vocableIds, std::inserter(setVocs, setVocs.begin()));
    }
    for (auto [length, count] : length_count) {
        fmt::print("VocSize: {}, count: {}\n", length, count);
    }
    return result;
}

auto cardsPrepending(uint vocId,
                     const std::map<uint, uint>& vocId_cardId,
                     const std::map<uint, CardMeta>& id_cm,
                     const std::set<uint>& setVocs) -> std::set<uint> {
    std::set<uint> necessaryVocs;
    std::set<uint> result;
    std::set<uint> cardsInProgress;
    uint cardIdGoal = vocId_cardId.at(vocId);
    cardsInProgress.insert(cardIdGoal);
    for (uint cardId : PopFront(cardsInProgress)) {
        result.insert(cardId);
        const auto& vocIds = id_cm.at(cardId).vocableIds;
        for (uint vidNow : vocIds) {
            uint nextCard = vocId_cardId.at(vidNow);
            if (not result.contains(nextCard))
                cardsInProgress.insert(nextCard);
        }
    }
    return result;
}

void VocabularySR_TreeWalker::ProcessGroup(Group& group) {
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
        fmt::print("card_id: id {}, voc_count {}\n", cid, cm.vocableIds.size());
    }
    fmt::print("--------------------------------------------\n");

    // std::map<uint, uint> vc_whoDeclares;
    std::set<uint> setVocs;
    std::set<uint> layerVocs;

    std::map<uint, std::set<uint>> length_cardIds;
    for (const auto& [cardId, cardMeta] : group.id_cardMeta) {
        length_cardIds[cardMeta.vocableIds.size()].insert(cardId);
    }
    fmt::print("Cards with length 4: {}\n", length_cardIds[4].size());

    std::set<uint> layer_cards;
    for (uint cardId : length_cardIds[4]) {
        const auto& vocIds = group.id_cardMeta.at(cardId).vocableIds;
        if (ranges::set_intersection(vocIds, layerVocs, utl::counting_iterator{}).out.count == 0) {
            ranges::copy(vocIds, std::inserter(layerVocs, layerVocs.begin()));
            layer_cards.insert(cardId);
        }
    }
    fmt::print("Cards with length 4 layer 1: {}\n", layer_cards.size());

    int counter = 0;

    std::map<uint, uint> vocId_cardId;

    auto unrefined = CardsBestSize(group.id_cardMeta);
    for (auto& [cardId, cardMeta] : unrefined) {
        for (uint vocId : cardMeta.vocableIds) {
            if (not vocId_cardId.contains(vocId))
                vocId_cardId[vocId] = cardId;
        }
        counter++;
        // if (counter == 10)
        //     break;
    }
    fmt::print("Counted {} cards\n", counter);

    std::map<uint, std::pair<uint, std::set<float>>> prependingCount;
    // for (const uint vocId : vocId_cardId | std::views::keys) {
    //     counter = 0;
    //     for (uint cid : cardsPrepending(vocId, vocId_cardId, group.id_cardMeta, setVocs))
    //         counter++;
    //     prependingCount[counter]++;
    // }
    // for (const auto [count, pre] : prependingCount) {
    //     fmt::print("Prepending: {}, count: {}\n", count, pre);
    // }
    fmt::print("------------------\n");
    prependingCount.clear();
    for (const uint vocId : vocId_cardId | std::views::keys) {
        counter = cardsPrepending(vocId, vocId_cardId, group.id_cardMeta, setVocs).size();

        auto& [pre, urgency] = prependingCount[counter];
        urgency.insert(id_vocableSR.at(vocId).urgency());
        pre++;
    }
    for (const auto& [count, pre] : prependingCount) {
        fmt::print("Prepending: {}, count: {}, urg: {}\n",
                   count,
                   pre.first,
                   fmt::join(AtMostTake(pre.second, 10), ", "));
    }
    fmt::print(
        "#######################################################################################\n");

    std::map<uint, CardMeta> id_cm2;
    ranges::copy(unrefined, std::inserter(id_cm2, id_cm2.begin()));
    auto refined = RefinedCards(unrefined);
    counter = 0;
    vocId_cardId.clear();
    for (auto& [cardId, cardMeta] : refined) {
        for (uint vocId : cardMeta.vocableIds) {
            if (not vocId_cardId.contains(vocId))
                vocId_cardId[vocId] = cardId;
        }
        counter++;
        // if (counter == 10)
        //     break;
    }
    fmt::print("Counted {} cards\n", counter);
    prependingCount.clear();
    for (const uint vocId : vocId_cardId | std::views::keys) {
        counter = cardsPrepending(vocId, vocId_cardId, group.id_cardMeta, setVocs).size();

        auto& [pre, urgency] = prependingCount[counter];
        urgency.insert(id_vocableSR.at(vocId).urgency());
        pre++;
    }
    for (const auto& [count, pre] : prependingCount) {
        fmt::print("Prepending: {}, count: {}, urg: {}\n",
                   count,
                   pre.first,
                   fmt::join(AtMostTake(pre.second, 10), ", "));
    }
}
