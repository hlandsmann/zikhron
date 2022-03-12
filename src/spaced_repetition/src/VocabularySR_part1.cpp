#include <VocabularySR.h>
#include <annotation/Markup.h>
#include <annotation/TextCard.h>
#include <fmt/ostream.h>
#include <spdlog/spdlog.h>
#include <utils/StringU8.h>
#include <utils/counting_iterator.h>
#include <utils/min_element_val.h>
#include <algorithm>
#include <boost/range/combine.hpp>
#include <chrono>
#include <cmath>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <nlohmann/json.hpp>
#include <optional>
#include <ranges>
#include <set>
#include <type_traits>

namespace ranges = std::ranges;

VocabularySR::~VocabularySR() {
    try {
        SaveProgress();
        SaveAnnotationChoices();
        SaveVocableChoices();
    }
    catch (const std::exception& e) {
        std::cout << e.what() << "\n";
    }
}

VocabularySR::VocabularySR(CardDB&& _cardDB, std::shared_ptr<ZH_Dictionary> _zh_dictionary)
    : cardDB(std::make_shared<CardDB>(std::move(_cardDB))), zh_dictionary(_zh_dictionary) {
    std::set<ZH_Annotator::Item> myDic;
    LoadVocableChoices();
    LoadAnnotationChoices();
    auto start = std::chrono::high_resolution_clock::now();
    GenerateFromCards();
    auto gfc = std::chrono::high_resolution_clock::now();
    LoadProgress();
    auto lp = std::chrono::high_resolution_clock::now();
    GenerateToRepeatWorkload();
    auto gtr = std::chrono::high_resolution_clock::now();
    namespace chrono = std::chrono;
    using microseconds = std::chrono::microseconds;
    fmt::print("Time for GenerateFromCards: {} \n",
               chrono::duration_cast<microseconds>(gfc - start).count());
    fmt::print("Time for LoadProgress: {} \n", chrono::duration_cast<microseconds>(lp - gfc).count());
    fmt::print("Time for GenerateToRepeatWorkload: {} \n",
               chrono::duration_cast<microseconds>(gtr - lp).count());
    fmt::print("Time totally elapsed: {} \n", chrono::duration_cast<microseconds>(gtr - start).count());

    CleanUpVocables();
    fmt::print("New Vocables that where not yet seen: {}\n", CountTotalNewVocablesInSet());
    // treeWalker = std::make_unique<VocabularySR_TreeWalker>(id_vocableSR, id_cardMeta, id_vocableMeta);
    X = std::make_unique<VocabularySR_X>(id_vocableSR, id_cardMeta, id_vocableMeta);
}
auto VocabularySR::CountTotalNewVocablesInSet() -> size_t {
    std::set<uint> newVocables;
    for (const auto& [id, cardMeta] : id_cardMeta) {
        ranges::set_difference(cardMeta.vocableIds,
                               id_vocableSR | std::views::keys,
                               std::inserter(newVocables, newVocables.begin()));
    }
    return newVocables.size();
}

void VocabularySR::CleanUpVocables() {
    std::set<uint> badVocableIds;
    std::set<uint> activeVocableIds;

    auto seenCards = id_cardMeta | std::views::filter([this](std::pair<uint, CardMeta> id_cm) {
                         return ranges::set_difference(id_cm.second.vocableIds,
                                                       id_vocableSR | std::views::keys,
                                                       utl::counting_iterator{})
                                    .out.count == 0;
                     });

    for (const auto& [id, card] : seenCards) {
        activeVocableIds.insert(card.vocableIds.begin(), card.vocableIds.end());
    }
    ranges::set_difference(id_vocableSR | std::views::keys,
                           activeVocableIds,
                           std::inserter(badVocableIds, badVocableIds.begin()));
    for (uint id : badVocableIds) {
        fmt::print("Erasing id: {} - key: {}\n",
                   id,
                   id_vocable.find(id) != id_vocable.end() ? id_vocable[id].front().key : "");
        id_vocableSR.erase(id);
        ids_repeatTodayVoc.erase(id);
        ids_againVoc.erase(id);
        ids_nowVoc.erase(id);
    }
}

void VocabularySR::GenerateFromCards() {
    const std::map<uint, CardDB::CardPtr>& cards = cardDB->get();

    for (const auto& [cardId, card] : cards) {
        utl::StringU8 card_text = markup::Paragraph::textFromCard(*card);
        card->zh_annotator.emplace(card_text, zh_dictionary, annotationChoices);
        InsertVocabulary(cardId);
    }
    fmt::print("Count of vocables: {}\n", zhdic_vocableMeta.size());
    std::set<std::string> allCharacters;

    for (const auto& vocable_key : zhdic_vocableMeta | std::views::keys) {
        if (vocable_key.empty())
            continue;

        const auto word = utl::StringU8(vocable_key);
        allCharacters.insert(word.cbegin(), word.cend());
    }

    fmt::print("Characters: {}\n", fmt::join(allCharacters, ""));
    fmt::print("Count of Characters: {}\n", allCharacters.size());
    fmt::print("VocableSize: {}\n", zhdic_vocableMeta.size());
}

auto VocabularySR::CalculateCardValueSingle(const CardMeta& cm, const std::set<uint>& good) const
    -> float {
    std::set<uint> cardIdsSharedVoc;
    int count = 0;

    const auto& setVocIdThis = cm.vocableIds;

    count += int(std::pow(
        float(ranges::set_intersection(setVocIdThis, good, utl::counting_iterator{}).out.count), 2.f));

    if (cardIdsSharedVoc.empty())
        return 1.f;

    return float(count) / float(setVocIdThis.size());
}

auto VocabularySR::CalculateCardValueSingleNewVoc(const CardMeta& cm,
                                                  const std::set<uint>& neutral) const -> float {
    std::set<uint> diff;
    ranges::set_difference(cm.vocableIds, neutral, std::inserter(diff, diff.begin()));
    size_t relevance = 0;
    for (uint vocId : diff) {
        relevance += id_vocableMeta.at(vocId).cardIds.size();
    }

    return float(relevance) / std::pow(std::abs(float(diff.size() - 2)) + 1.f, float(diff.size()));
}

void VocabularySR::InsertVocabulary(uint cardId) {
    CardMeta& cm = id_cardMeta[cardId];
    const CardDB::CardPtr& cardPtr = cardDB->get().at(cardId);
    const ZH_Annotator& annotator = cardPtr->zh_annotator.value();

    // Its unfortunate, that we cannot simply use a view.... but we gotta live with that.
    // So lets create a temporary vector annotatorItems to represent that view.
    std::vector<std::reference_wrapper<const ZH_Annotator::ZH_dicItemVec>> annotatorItems;
    ranges::transform(annotator.Items() | std::views::filter([](const ZH_Annotator::Item& item) {
                          return not item.dicItemVec.empty();
                      }),
                      std::back_inserter(annotatorItems),
                      [](const auto& item) -> std::reference_wrapper<const ZH_Annotator::ZH_dicItemVec> {
                          return item.dicItemVec;
                      });

    std::vector<uint> vocableIds = GetVocableIdsInOrder(cardId);
    for (const auto& [vocId, dicItemVec] : boost::combine(vocableIds, annotatorItems)) {
        const auto& dicEntry = zh_dictionary->EntryFromPosition(vocId, zh_dictionary->Simplified());
        const auto& key = dicEntry.key;

        if (auto it = zhdic_vocableMeta.find(key); it != zhdic_vocableMeta.end()) {
            auto& vocableMeta = id_vocableMeta[vocId];
            vocableMeta.cardIds.insert(cardId);

            cm.vocableIds.insert(vocId);
        } else {
            id_vocableMeta[vocId] = {.cardIds = {cardId}};
            zhdic_vocableMeta[key] = vocId;
            id_vocable[vocId] = ZH_dicItemVec{dicItemVec};

            cm.vocableIds.insert(vocId);
        }
    }
    cm.cardId = cardId;
}

void VocabularySR::EraseVocabulary(uint cardId) {
    CardMeta& cm = id_cardMeta.at(cardId);
    for (uint vocId : cm.vocableIds) {
        auto& vocable = id_vocableMeta.at(vocId);
        vocable.cardIds.erase(cardId);
    }
    cm.vocableIds.clear();
}

auto VocabularySR::GetCardRepeatedVoc() -> std::optional<uint> {
    struct intersect_view {
        std::set<uint> intersectToday{};
        size_t countIntersectionNow{};

        uint viewCount{};
    };
    std::map<uint, intersect_view> candidates;
    for (const auto& [id, cardMeta] : id_cardMeta) {
        size_t count_union = ranges::set_union(id_vocableSR | std::views::keys,
                                               cardMeta.vocableIds,
                                               utl::counting_iterator{})
                                 .out.count;
        if (count_union != id_vocableSR.size())
            continue;
        std::set<uint> intersectToday;
        ranges::set_intersection(ids_repeatTodayVoc,
                                 cardMeta.vocableIds,
                                 std::inserter(intersectToday, intersectToday.begin()));
        size_t count_now = ranges::set_intersection(
                               ids_nowVoc, cardMeta.vocableIds, utl::counting_iterator{})
                               .out.count;
        if (intersectToday.empty() && count_now == 0)
            continue;
        uint view_count{};
        if (const auto& it = id_cardSR.find(id); it != id_cardSR.end())
            view_count = it->second.viewCount;
        candidates[id] = {.intersectToday = intersectToday,
                          .countIntersectionNow = count_now,
                          .viewCount = view_count};
    }
    if (candidates.empty()) {
        for (uint id : ids_repeatTodayVoc) {
            id_vocableSR.erase(id);

            fmt::print("Erasing id: {} - key: {}\n", id, id_vocable[id].front().key);
        }
        ids_repeatTodayVoc.clear();
        return {};
    }

    const auto preferedQuantity = [](size_t a, size_t b) -> bool {
        const std::array quantity = {4, 3, 5, 2, 6};
        const auto a_it = ranges::find(quantity, a);
        const auto b_it = ranges::find(quantity, b);
        if (a_it != b_it)
            return a_it > b_it;
        return a > b;
    };
    auto urgency = [this](uint vocId) { return id_vocableSR.at(vocId).urgency(); };
    return ranges::max_element(
               candidates,
               [&preferedQuantity, &urgency](const intersect_view& a, const intersect_view& b) {
                   if (a.countIntersectionNow != b.countIntersectionNow)
                       return a.countIntersectionNow < b.countIntersectionNow;
                   if (a.intersectToday.size() <= 5 && b.intersectToday.size() <= 5) {
                       float a_urgency = utl::min_element_val(a.intersectToday,
                                                              std::numeric_limits<float>::infinity(),
                                                              ranges::less{},
                                                              urgency);
                       float b_urgency = utl::min_element_val(b.intersectToday,
                                                              std::numeric_limits<float>::infinity(),
                                                              ranges::less{},
                                                              urgency);
                       if ((a_urgency <= 5 || b_urgency <= 5) && a_urgency != b_urgency)
                           return a_urgency > b_urgency;
                   }
                   if (a.intersectToday.size() != b.intersectToday.size())
                       return preferedQuantity(a.intersectToday.size(), b.intersectToday.size());

                   return a.viewCount > b.viewCount;
               },
               &decltype(candidates)::value_type::second)
        ->first;
}

auto VocabularySR::GetCardNewVocStart() -> std::optional<uint> {
    if (countOfNewVocablesToday > 0)
        return {};
    if (ids_againVoc.size() >= 9) {
        fmt::print("Vocables that failed are of quantity {}. Therefore no new vocables for now\n",
                   ids_againVoc.size());
        return {};
    }
    struct intersections {
        size_t countRepeat{};
        size_t countNew{};
        size_t valueNewAccumulated{};
    };
    std::map<uint, intersections> candidates;

    constexpr int maxNewPerCard = 6;
    constexpr int maxRepeatPerNewCard = 2;

    for (const auto& [id, cardMeta] : id_cardMeta) {
        size_t countNew = ranges::set_difference(cardMeta.vocableIds,
                                                 id_vocableSR | std::views::keys,
                                                 utl::counting_iterator{})
                              .out.count;
        if (countNew == 0 || countNew > maxNewPerCard)
            continue;
        size_t countRepeat = ranges::set_intersection(
                                 cardMeta.vocableIds, ids_repeatTodayVoc, utl::counting_iterator{})
                                 .out.count;
        if (countRepeat > maxRepeatPerNewCard)
            continue;

        size_t valueNewAccumulated = std::accumulate(cardMeta.vocableIds.begin(),
                                                     cardMeta.vocableIds.end(),
                                                     size_t{},
                                                     [&](const size_t val, const uint vocId) {
                                                         if (id_vocableSR.contains(vocId))
                                                             return val;
                                                         return val +
                                                                id_vocableMeta.at(vocId).cardIds.size();
                                                     });
        candidates[id] = {.countRepeat = countRepeat,
                          .countNew = countNew,
                          .valueNewAccumulated = valueNewAccumulated};
    }
    if (candidates.empty()) {
        fmt::print("No candidates found with new words\n");
        return {};
    }
    auto choice = ranges::max_element(
        candidates, std::less{}, [](const auto& c) { return c.second.valueNewAccumulated; });
    fmt::print("Got {} candidates for new vocables. Choice value: {}, new words: {}, newToday: {}\n",
               candidates.size(),
               choice->second.valueNewAccumulated,
               choice->second.countNew,
               countOfNewVocablesToday);

    countOfNewVocablesToday += choice->second.countNew;
    return choice->first;
}

auto VocabularySR::GetCardNewVoc() -> std::optional<uint> {
    using id_cardMeta_v_t = typename decltype(id_cardMeta)::value_type;
    std::function<float(id_cardMeta_v_t&)> calcValue;

    if (ids_repeatTodayVoc.empty()) {
        std::set<uint> studiedVocableIds;
        ranges::copy(id_vocableSR | std::views::keys,
                     std::inserter(studiedVocableIds, studiedVocableIds.begin()));
        calcValue = [&, studiedVocableIds](const id_cardMeta_v_t& id_cm) -> float {
            return CalculateCardValueSingleNewVoc(id_cm.second, studiedVocableIds);
        };
    } else {
        calcValue = [&](const id_cardMeta_v_t& id_cm) -> float {
            return CalculateCardValueSingle(id_cm.second, ids_repeatTodayVoc);
        };
    }

    const auto& it_id_cm = ranges::max_element(id_cardMeta, ranges::less{}, calcValue);
    if (it_id_cm == id_cardMeta.end())
        return {};
    const CardMeta& cm = it_id_cm->second;

    return cm.cardId;
}

auto VocabularySR::getCard() -> std::tuple<std::unique_ptr<Card>, VocableIds_vt, Id_Ease_vt> {
    if (getCardNeedsCleanup) {
        getCardNeedsCleanup = false;
        fmt::print("Cleaning up vocables .......\n");
        CleanUpVocables();
    }

    std::set<uint> toRepeatVoc;
    ranges::copy_if(ids_againVoc, std::inserter(toRepeatVoc, toRepeatVoc.begin()), [&](uint id) {
        return id_vocableSR.at(id).pauseTimeOver();
    });
    for (uint id : toRepeatVoc) {
        ids_againVoc.erase(id);
        ids_repeatTodayVoc.insert(id);
        ids_nowVoc.insert(id);
        fmt::print("Wait time over for {}\n", id_vocable.at(id).front().key);
    }
    fmt::print("To Repeat: {}, Again: {}\n", ids_repeatTodayVoc.size(), ids_againVoc.size());

    if (not ids_nowVoc.empty()) {
        auto repeatVocStart = GetCardRepeatedVoc();
        assert(repeatVocStart.has_value());
        activeCardId = repeatVocStart.value();
        fmt::print("Get Card with words that are to be repeated now id: {}\n", activeCardId);
    } else if (auto cardNewVocStart = GetCardNewVocStart(); cardNewVocStart.has_value()) {
        activeCardId = cardNewVocStart.value();
        fmt::print("Get Card with new vocables for starters#{}\n", activeCardId);
    } else if (auto cardRepeatVoc = GetCardRepeatedVoc(); cardRepeatVoc.has_value()) {
        activeCardId = cardRepeatVoc.value();
        fmt::print("Get Card with repeated vocables #{}\n", activeCardId);
    } else if (auto cardNewVoc = GetCardNewVoc(); cardNewVoc.has_value()) {
        activeCardId = cardNewVoc.value();
        fmt::print("Get new words from Card #{}\n", activeCardId);
    } else
        return {nullptr, VocableIds_vt{}, Id_Ease_vt{}};
    // activeCardId = 795;
    std::vector<std::string> advancedVocables;
    std::vector<std::string> unchangedVocables;
    for (uint vocId : id_cardMeta.at(activeCardId).vocableIds)
        if (auto it = id_vocableSR.find(vocId); it != id_vocableSR.end()) {
            if (it->second.advanceIndirectly())
                advancedVocables.push_back(id_vocable.at(it->first).front().key);
            else
                unchangedVocables.push_back(id_vocable.at(it->first).front().key);
        }

    fmt::print("Advancing indirectly: {}\n", fmt::join(advancedVocables, ", "));
    fmt::print("Unchanged are: {}\n", fmt::join(unchangedVocables, ", "));

    return {cardDB->get().at(activeCardId)->clone(),
            GetVocableIdsInOrder(activeCardId),
            GetRelevantEase(activeCardId)};
}

auto VocabularySR::GetVocableIdsInOrder(uint cardId) const -> std::vector<uint> {
    const CardDB::CardPtr& cardPtr = cardDB->get().at(cardId);
    assert(cardPtr->zh_annotator.has_value());
    const ZH_Annotator& annotator = cardPtr->zh_annotator.value();
    std::vector<uint> vocableIds;
    ranges::transform(annotator.Items() | std::views::filter([](const ZH_Annotator::Item& item) {
                          return not item.dicItemVec.empty();
                      }),
                      std::back_inserter(vocableIds),
                      [this](const ZH_Annotator::Item& item) -> uint {
                          uint vocId = item.dicItemVec.front().id;
                          if (const auto it = id_id_vocableChoices.find(vocId);
                              it != id_id_vocableChoices.end())
                              vocId = it->second;
                          return vocId;
                      });
    return vocableIds;
}
