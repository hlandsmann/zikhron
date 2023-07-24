#include <Vocabulary.h>
#include <annotation/Markup.h>
#include <annotation/TextCard.h>
#include <annotation/ZH_Annotator.h>
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
#include <map>
#include <nlohmann/json.hpp>
#include <optional>
#include <ranges>
#include <set>
#include <type_traits>

namespace ranges = std::ranges;

VocabularySR::VocabularySR(const std::shared_ptr<CardDB>& cardDB_in,
                           const std::shared_ptr<ZH_Dictionary>& zh_dictionary_in)
    : cardDB(cardDB_in), zh_dictionary(zh_dictionary_in), sr_db(cardDB, zh_dictionary) {
    spdlog::info("New Vocables that where not yet seen: {}\n", CountTotalNewVocablesInSet());
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

    return float{relevance} / std::pow(std::abs(float(diff.size() - 2)) + 1.f, float(diff.size()));
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
            spdlog::error("Id: {} should have been studied, but no card found\n", id);
        }
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
    // return {};

    if (countOfNewVocablesToday > 20)
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

    constexpr int maxNewPerCard = 8;
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
    using id_cardMeta_v_t = typename std::remove_reference_t<decltype(id_cardMeta)>::value_type;
    std::function<float(const id_cardMeta_v_t&)> calcValue;

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

    const auto it_id_cm = ranges::max_element(id_cardMeta, ranges::less{}, calcValue);
    if (it_id_cm == id_cardMeta.end())
        return {};
    const CardMeta& cm = it_id_cm->second;

    return cm.cardId;
}

auto VocabularySR::getNextCardChoice(std::optional<uint> preferedCardId)
    -> std::tuple<std::unique_ptr<Card>, VocableIds_vt, Id_Ease_vt> {
    sr_db.AdvanceFailedVocables();
    fmt::print("To Repeat: {}, Again: {}\n", ids_repeatTodayVoc.size(), ids_againVoc.size());

    if (preferedCardId.has_value()) {
        activeCardId = *preferedCardId;
    } else if (not ids_nowVoc.empty()) {
        auto repeatVocStart = GetCardRepeatedVoc();
        assert(repeatVocStart.has_value());
        activeCardId = *repeatVocStart;
        fmt::print("Get Card with words that are to be repeated now id: {}\n", *activeCardId);
    } else if (auto cardNewVocStart = GetCardNewVocStart(); cardNewVocStart.has_value()) {
        activeCardId = *cardNewVocStart;
        fmt::print("Get Card with new vocables for starters#{}\n", *activeCardId);
    } else if (auto cardRepeatVoc = GetCardRepeatedVoc(); cardRepeatVoc.has_value()) {
        activeCardId = *cardRepeatVoc;
        fmt::print("Get Card with repeated vocables #{}\n", *activeCardId);
    } else if (auto cardNewVoc = GetCardNewVoc(); cardNewVoc.has_value()) {
        activeCardId = *cardNewVoc;
        fmt::print("Get new words from Card #{}\n", *activeCardId);
    } else
        return {nullptr, VocableIds_vt{}, Id_Ease_vt{}};
    // activeCardId = 795;

    return {cardDB->get().at(*activeCardId)->clone(),
            sr_db.GetVocableIdsInOrder(*activeCardId),
            GetRelevantEase(*activeCardId)};
}

auto VocabularySR::getCardFromId(uint id) const
    -> std::optional<std::tuple<std::unique_ptr<Card>, VocableIds_vt, Id_Ease_vt>> {
    if (cardDB->get().contains(id))
        return {{cardDB->get().at(id)->clone(), sr_db.GetVocableIdsInOrder(id), GetRelevantEase(id)}};
    else
        return {};
}

auto VocabularySR::AddAnnotation(const ZH_Annotator::Combination& combination,
                                 const std::vector<utl::CharU8>& characterSequence) -> CardInformation {
    sr_db.AddAnnotation(combination, characterSequence, *activeCardId);
    const auto& card = cardDB->get().at(*activeCardId);
    return {std::unique_ptr<Card>(card->clone()),
            sr_db.GetVocableIdsInOrder(*activeCardId),
            GetRelevantEase(*activeCardId)};
}

auto VocabularySR::AddVocableChoice(uint vocId, uint vocIdOldChoice, uint vocIdNewChoice)
    -> CardInformation {
    sr_db.AddVocableChoice(vocId, vocIdOldChoice, vocIdNewChoice);

    const auto& card = cardDB->get().at(*activeCardId);
    return {std::unique_ptr<Card>(card->clone()),
            sr_db.GetVocableIdsInOrder(*activeCardId),
            GetRelevantEase(*activeCardId)};
}

auto VocabularySR::GetActiveVocables(uint cardId) const -> std::set<uint> {
    std::set<uint> activeVocables;
    const CardMeta& cm = id_cardMeta.at(cardId);

    auto vocableActive = [&](uint vocId) -> bool {
        return (not id_vocableSR.contains(vocId)) || id_vocableSR.at(vocId).isToBeRepeatedToday() ||
               ids_repeatTodayVoc.contains(vocId) || ids_againVoc.contains(vocId);
    };
    ranges::copy_if(cm.vocableIds, std::inserter(activeVocables, activeVocables.begin()), vocableActive);
    return activeVocables;
}

auto VocabularySR::GetRelevantEase(uint cardId) const -> Id_Ease_vt {
    std::set<uint> activeVocables = GetActiveVocables(cardId);
    Id_Ease_vt ease;
    ranges::transform(
        activeVocables, std::inserter(ease, ease.begin()), [&](uint vocId) -> Id_Ease_vt::value_type {
            const VocableSR vocSR = id_vocableSR.contains(vocId) ? id_vocableSR.at(vocId) : VocableSR();
            spdlog::debug("Easefactor of {} is {:.2f}, invervalDay {:.2f} - id: {}",
                          zh_dictionary->EntryFromPosition(vocId, zh_dictionary->Simplified()).key,
                          vocSR.EaseFactor(),
                          vocSR.IntervalDay(),
                          vocId);
            return {vocId, {vocSR.IntervalDay(), vocSR.EaseFactor(), vocSR.IndirectIntervalDay()}};
        });
    return ease;
}

void VocabularySR::setEaseLastCard(const Id_Ease_vt& id_ease) {
    for (auto [vocId, ease] : id_ease) {
        sr_db.SetEase(vocId, ease);
    }
    sr_db.ViewCard(*activeCardId);
    sr_db.AdvanceIndirectlySeenVocables(*activeCardId);
}
