#include "VocabularySR.h"
#include <TextCard.h>
#include <fmt/ostream.h>
#include <utils/Markup.h>
#include <utils/StringU8.h>
#include <algorithm>
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
    } catch (const std::exception& e) { std::cout << e.what() << "\n"; }
}
VocabularySR::VocabularySR(CardDB&& _cardDB, std::shared_ptr<ZH_Dictionary> _zh_dictionary)
    : cardDB(std::make_shared<CardDB>(std::move(_cardDB))), zh_dictionary(_zh_dictionary) {
    std::set<ZH_Annotator::Item> myDic;

    GenerateFromCards();
    LoadProgress();
    GenerateToRepeatWorkload();
}

void VocabularySR::GenerateFromCards() {
    const std::map<uint, CardDB::CardPtr>& cards = cardDB->get();
    for (const auto& [cardId, card] : cards) {
        utl::StringU8 card_text = markup::Paragraph::textFromCard(*card);
        ZH_Annotator annotator = ZH_Annotator(card_text, zh_dictionary);

        InsertVocabulary(annotator.UniqueItems(), cardId);
    }
    std::cout << "Size: " << zhdic_vocableMeta.size();
    // for (const auto& [voc, vocmeta] : vocables) {
    //     std::cout << voc.front().key << " : ";
    //     std::cout << vocmeta.id << " - ";
    //     for (const auto& num : vocmeta.cardIds)
    //         std::cout << num << ", ";
    //     std::cout << "\n";
    // }

    for (const auto& voc : zhdic_vocableMeta) {
        if (voc.first.empty())
            continue;

        const auto word = utl::StringU8(voc.first.front().key);
        allCharacters.insert(word.cbegin(), word.cend());
    }
    for (const auto& mychar : allCharacters) {
        std::cout << mychar;
    }
    std::cout << "\n";
    std::cout << "Count of Characters: " << allCharacters.size() << "\n";
    std::cout << "VocableSize: " << zhdic_vocableMeta.size() << "\n";
}

auto VocabularySR::CalculateCardValueSingle(const CardMeta& cm, const std::set<uint>& good) const
    -> float {
    std::set<uint> cardIdsSharedVoc;
    int count = 0;

    // for (uint vocId : cm.vocableIds) {
    //     const VocableMeta& vm = id_vocableMeta.at(vocId);
    //     ranges::copy(vm.cardIds, std::inserter(cardIdsSharedVoc, cardIdsSharedVoc.begin()));
    // }

    const auto& setVocIdThis = cm.vocableIds;
    // for (uint cId : cardIdsSharedVoc) {
    //     const auto& setVocOther = id_cardMeta.at(cId)->vocableIds;
    count += pow(
        std::set_intersection(
            setVocIdThis.begin(), setVocIdThis.end(), good.begin(), good.end(), counting_iterator{})
            .count,
        2);
    // }
    if (cardIdsSharedVoc.empty())
        return 1.;

    return float(count) / float(setVocIdThis.size());
}

auto VocabularySR::CalculateCardValueSingleNewVoc(const CardMeta& cm,
                                                  const std::set<uint>& neutral) const -> float {
    std::set<uint> diff;
    std::set_difference(cm.vocableIds.begin(),
                        cm.vocableIds.end(),
                        neutral.begin(),
                        neutral.end(),
                        std::inserter(diff, diff.begin()));
    int relevance = 0;
    for (uint vocId : diff) {
        relevance += id_vocableMeta.at(vocId).cardIds.size();
    }

    return float(relevance) / pow(abs(float(diff.size() - 2)) + 1, diff.size());
}

void VocabularySR::InsertVocabulary(const std::set<ZH_Annotator::Item>& cardVocabulary, uint cardId) {
    CardMeta& cm = id_cardMeta[cardId];

    for (auto& item : cardVocabulary) {
        if (auto it = zhdic_vocableMeta.find(item.dicItemVec); it != zhdic_vocableMeta.end()) {
            uint vocId = it->second;
            auto& vocable = id_vocableMeta[vocId];
            vocable.cardIds.insert(cardId);

            cm.vocableIds.insert(vocId);
        } else {
            uint vocId = item.dicItemVec.front().id;
            id_vocableMeta[vocId] = {.cardIds = {cardId}};
            zhdic_vocableMeta[item.dicItemVec] = vocId;
            id_vocable[vocId] = ZH_dicItemVec{item.dicItemVec};

            cm.vocableIds.insert(vocId);
        }
    }
    cm.cardId = cardId;
}

auto VocabularySR::GetCardRepeatedVoc() -> std::optional<uint> {
    struct intersect_view {
        size_t countIntersect{};
        size_t countIntersectionNow{};

        uint viewCount{};
    };
    std::map<uint, intersect_view> candidates;
    for (const auto& [id, cardMeta] : id_cardMeta) {  //   std::set<uint> test;
        size_t count_union =
            ranges::set_union(id_vocableSR | std::views::keys, cardMeta.vocableIds, counting_iterator{})
                .out.count;
        if (count_union != id_vocableSR.size())
            continue;
        size_t count_intersect = ranges::set_intersection(
                                     ids_repeatTodayVoc, cardMeta.vocableIds, counting_iterator{})
                                     .out.count;
        size_t count_now =
            ranges::set_intersection(ids_nowVoc, cardMeta.vocableIds, counting_iterator{}).out.count;
        if (count_intersect == 0 && count_now == 0)
            continue;
        uint view_count{};
        if (const auto& it = id_cardSR.find(id); it != id_cardSR.end())
            view_count = it->second.viewCount;
        candidates[id] = {.countIntersect = count_intersect,
                          .countIntersectionNow = count_now,
                          .viewCount = view_count};
    }
    if (candidates.empty()) {
        for (uint id : ids_repeatTodayVoc) {
            id_vocableSR.erase(id);
            fmt::print("Erasing id: {} - key: {}\n", id, id_vocable[id].front().key);
        }
        return {};
    }

    return ranges::min_element(
               candidates,
               [](const intersect_view& a, const intersect_view& b) {
                   if (a.countIntersectionNow != b.countIntersectionNow)
                       return a.countIntersectionNow > b.countIntersectionNow;
                   if (a.viewCount == b.viewCount)
                       return a.countIntersect < b.countIntersect;
                   if (std::abs(int(a.countIntersect) - int(b.countIntersect)) <= 1)
                       return a.viewCount < b.viewCount;
                   return a.countIntersect < b.countIntersect;
               },
               &decltype(candidates)::value_type::second)
        ->first;
}

auto VocabularySR::GetCardNewVocStart() -> std::optional<uint> {
    if (countOfNewVocablesToday > 42)
        return {};
    if (ids_againVoc.size() >= 9) {
        fmt::print("Vocables that failed are of quantity {}. Therefore no new vocables for now\n", ids_againVoc.size());
        return {};
    }
    struct intersections {
        size_t countRepeat{};
        size_t countNew{};
        size_t valueNewAccumulated{};
    };
    std::map<uint, intersections> candidates;

    constexpr int maxNewPerCard = 3;
    constexpr int maxRepeatPerNewCard = 6;

    for (const auto& [id, cardMeta] : id_cardMeta) {  //   std::set<uint> test;

        size_t countNew = ranges::set_difference(
                              cardMeta.vocableIds, id_vocableSR | std::views::keys, counting_iterator{})
                              .out.count;
        if (countNew == 0 || countNew > maxNewPerCard)
            continue;
        size_t countRepeat = ranges::set_intersection(
                                 cardMeta.vocableIds, ids_repeatTodayVoc, counting_iterator{})
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
    return {};
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
auto VocabularySR::getCard() -> std::tuple<std::unique_ptr<Card>, Item_Id_vt, Id_Ease_vt> {
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
    uint cardId;
    if (not ids_nowVoc.empty()) {
        auto repeatVocStart = GetCardRepeatedVoc();
        assert(repeatVocStart.has_value());
        cardId = repeatVocStart.value();
        fmt::print("Get Card with words that are to be repeated now id: {}\n", cardId);
    } else if (auto cardNewVocStart = GetCardNewVocStart(); cardNewVocStart.has_value()) {
        cardId = cardNewVocStart.value();
        fmt::print("Get Card with new vocables for starters#{}\n", cardId);
    } else if (auto cardRepeatVoc = GetCardRepeatedVoc(); cardRepeatVoc.has_value()) {
        cardId = cardRepeatVoc.value();
        fmt::print("Get Card with repeated vocables #{}\n", cardId);
    } else if (auto cardNewVoc = GetCardNewVoc(); cardNewVoc.has_value()) {
        cardId = cardNewVoc.value();
        fmt::print("Get new words from Card #{}\n", cardId);
    } else
        return {nullptr, Item_Id_vt{}, Id_Ease_vt{}};

    id_cardSR[cardId].ViewNow();
    std::vector<std::string> advancedVocables;
    std::vector<std::string> unchangedVocables;
    for (uint vocId : id_cardMeta.at(cardId).vocableIds)
        if (auto it = id_vocableSR.find(vocId); it != id_vocableSR.end()) {
            if (it->second.advanceIndirectly())
                advancedVocables.push_back(id_vocable.at(it->first).front().key);
            else
                unchangedVocables.push_back(id_vocable.at(it->first).front().key);
        }

    fmt::print("Advancing indirectly: {}\n", fmt::join(advancedVocables, ", "));
    fmt::print("Unchanged are: {}\n", fmt::join(unchangedVocables, ", "));

    return {std::unique_ptr<Card>(cardDB->get().at(cardId)->clone()),
            GetRelevantVocables(cardId),
            GetRelevantEase(cardId)};
}

auto VocabularySR::GetActiveVocables(uint cardId) -> std::set<uint> {
    std::set<uint> activeVocables;
    const CardMeta& cm = id_cardMeta.at(cardId);

    auto vocableActive = [&](uint vocId) -> bool {
        return (not id_vocableSR.contains(vocId)) || ids_repeatTodayVoc.contains(vocId) ||
               ids_againVoc.contains(vocId);
    };
    ranges::copy_if(cm.vocableIds, std::inserter(activeVocables, activeVocables.begin()), vocableActive);
    return activeVocables;
}

auto VocabularySR::GetRelevantVocables(uint cardId) -> Item_Id_vt {
    std::set<uint> activeVocables = GetActiveVocables(cardId);

    Item_Id_vt relevantVocables;
    ranges::transform(
        activeVocables, std::back_inserter(relevantVocables), [&](uint vocId) -> Item_Id_vt::value_type {
            return {id_vocable.at(vocId).front(), vocId};
        });
    return relevantVocables;
}

auto VocabularySR::GetRelevantEase(uint cardId) -> Id_Ease_vt {
    std::set<uint> activeVocables = GetActiveVocables(cardId);
    Id_Ease_vt ease;
    ranges::transform(
        activeVocables, std::inserter(ease, ease.begin()), [&](uint vocId) -> Id_Ease_vt::value_type {
            const VocableSR& vocSR = id_vocableSR[vocId];
            float easeFactor = vocSR.easeFactor;
            float intervalDay = vocSR.intervalDay;
            fmt::print("Easefactor of {} is {:.2f}, invervalDay {:.2f} - id: {}\n",
                       id_vocable.at(vocId).front().key,
                       easeFactor,
                       intervalDay,
                       vocId);
            if (easeFactor <= 1.31 && intervalDay < 2)
                return {vocId, Ease::again};
            if (easeFactor <= 1.6 && intervalDay < 5)
                return {vocId, Ease::hard};
            if (easeFactor <= 2.1)
                return {vocId, Ease::good};
            return {vocId, Ease::easy};
        });
    return ease;
}

void VocabularySR::setEaseLastCard(const Id_Ease_vt& id_ease) {
    for (auto [id, ease] : id_ease) {
        VocableSR& vocableSR = id_vocableSR[id];

        vocableSR.advanceByEase(ease);
        ids_nowVoc.erase(id);

        if (ease == Ease::again)
            ids_againVoc.insert(id);
        else
            ids_againVoc.erase(id);

        ids_repeatTodayVoc.erase(id);

        fmt::print("Ease of {} is {}\n", id_vocable.at(id).front().key, mapEaseToInt(ease));
    }
}

void VocabularySR::SaveProgress() {
    auto saveMetaFile = [](const std::string_view& fn, const nlohmann::json& js) {
        namespace fs = std::filesystem;
        fs::path fn_metaFile = fs::path(s_path_meta) / fn;
        std::ofstream ofs(fn_metaFile);
        ofs << js.dump(4);
    };

    auto generateJsonFromMap = [](const auto& map) -> nlohmann::json {
        nlohmann::json jsonMeta = nlohmann::json::object();
        auto& content = jsonMeta[std::string(s_content)];
        content = nlohmann::json::array();

        using sr_t = typename std::decay_t<decltype(map)>::mapped_type;
        ranges::transform(map, std::back_inserter(content), &sr_t::toJson);
        return jsonMeta;
    };
    std::filesystem::create_directory(s_path_meta);

    // save file for VocableSR --------------------------------------------
    nlohmann::json jsonVocSR = generateJsonFromMap(id_vocableSR);
    saveMetaFile(s_fn_metaVocableSR, jsonVocSR);

    // save file for CardSR -----------------------------------------------
    nlohmann::json jsonCardSR = generateJsonFromMap(id_cardSR);
    saveMetaFile(s_fn_metaCardSR, jsonCardSR);
}

void VocabularySR::LoadProgress() {
    auto loadJsonFromFile = [](const std::string_view& fn) -> nlohmann::json {
        namespace fs = std::filesystem;
        fs::path fn_metaFile = fs::path(s_path_meta) / fn;
        std::ifstream ifs(fn_metaFile);
        return nlohmann::json::parse(ifs);
    };

    auto jsonToMap = [](auto& map, const nlohmann::json& jsonMeta) {
        const auto& content = jsonMeta.at(std::string(s_content));
        using sr_t = typename std::decay_t<decltype(map)>::mapped_type;
        ranges::transform(content, std::inserter(map, map.begin()), &sr_t::fromJson);
    };
    try {
        nlohmann::json jsonVocSR = loadJsonFromFile(s_fn_metaVocableSR);
        jsonToMap(id_vocableSR, jsonVocSR);
        fmt::print("Vocable SR file {} loaded!\n", s_fn_metaVocableSR);
    } catch (const std::exception& e) {
        fmt::print("Vocabulary SR load for {} failed! Exception {}", s_fn_metaVocableSR, e.what());
    }
    try {
        nlohmann::json jsonCardSR = loadJsonFromFile(s_fn_metaCardSR);
        jsonToMap(id_cardSR, jsonCardSR);
        fmt::print("Card SR file {} loaded!\n", s_fn_metaCardSR);
    } catch (const std::exception& e) {
        fmt::print("Card SR load for {} failed! Exception {}", s_fn_metaCardSR, e.what());
    }
}

void VocabularySR::GenerateToRepeatWorkload() {
    // for(auto& [vocId, vocSR]: id_vocableSR)
    ranges::transform(id_vocableSR | std::views::filter([](const auto& pair_id_vocSR) {
                          return pair_id_vocSR.second.isToBeRepeatedToday();
                      }),
                      std::inserter(ids_repeatTodayVoc, ids_repeatTodayVoc.begin()),
                      [](const auto& p) { return p.first; });
}
