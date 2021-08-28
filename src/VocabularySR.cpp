#include "VocabularySR.h"
#include <TextCard.h>
#include <fmt/ostream.h>
#include <utils/Markup.h>
#include <utils/StringU8.h>
#include <utils/min_element_val.h>
#include <algorithm>
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
    } catch (const std::exception& e) { std::cout << e.what() << "\n"; }
}

VocabularySR::VocabularySR(CardDB&& _cardDB, std::shared_ptr<ZH_Dictionary> _zh_dictionary)
    : cardDB(std::make_shared<CardDB>(std::move(_cardDB))), zh_dictionary(_zh_dictionary) {
    std::set<ZH_Annotator::Item> myDic;

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
    treeWalker = std::make_unique<VocabluarySR_TreeWalker>(id_vocableSR, id_cardMeta, id_vocableMeta);
}

void VocabularySR::CleanUpVocables() {
    std::set<uint> badVocableIds;
    std::set<uint> activeVocableIds;

    auto seenCards = id_cardMeta | std::views::filter([this](std::pair<uint, CardMeta> id_cm) {
                         return ranges::set_difference(id_cm.second.vocableIds,
                                                       id_vocableSR | std::views::keys,
                                                       counting_iterator{})
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

        InsertVocabulary(card->zh_annotator.value().UniqueItems(), cardId);
    }
    fmt::print("Count of vocables: {}\n", zhdic_vocableMeta.size());
    std::set<std::string> allCharacters;

    for (const auto& voc : zhdic_vocableMeta) {
        if (voc.first.empty())
            continue;

        const auto word = utl::StringU8(voc.first.front().key);
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

    count += pow(ranges::set_intersection(setVocIdThis, good, counting_iterator{}).out.count, 2);

    if (cardIdsSharedVoc.empty())
        return 1.;

    return float(count) / float(setVocIdThis.size());
}

auto VocabularySR::CalculateCardValueSingleNewVoc(const CardMeta& cm,
                                                  const std::set<uint>& neutral) const -> float {
    std::set<uint> diff;
    ranges::set_difference(cm.vocableIds, neutral, std::inserter(diff, diff.begin()));
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
    for (const auto& [id, cardMeta] : id_cardMeta) {  //   std::set<uint> test;
        size_t count_union =
            ranges::set_union(id_vocableSR | std::views::keys, cardMeta.vocableIds, counting_iterator{})
                .out.count;
        if (count_union != id_vocableSR.size())
            continue;
        std::set<uint> intersectToday;
        ranges::set_intersection(ids_repeatTodayVoc,
                                 cardMeta.vocableIds,
                                 std::inserter(intersectToday, intersectToday.begin()));
        size_t count_now =
            ranges::set_intersection(ids_nowVoc, cardMeta.vocableIds, counting_iterator{}).out.count;
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

    const auto preferedQuantity = [](int a, int b) -> bool {
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
    if (countOfNewVocablesToday > 60)
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

    constexpr int maxNewPerCard = 4;
    constexpr int maxRepeatPerNewCard = 2;

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
        return {nullptr, Item_Id_vt{}, Id_Ease_vt{}};

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

    return {std::unique_ptr<Card>(cardDB->get().at(activeCardId)->clone()),
            GetRelevantVocables(activeCardId),
            GetRelevantEase(activeCardId)};
}

auto VocabularySR::addAnnotation(const std::vector<int>& combination,
                                 const std::vector<utl::ItemU8>& characterSequence) -> CardInformation {
    annotationChoices[characterSequence] = combination;

    std::set<uint> cardsWithCharSeq;

    for (const auto& [id, cardPtr] : cardDB->get()) {
        if (cardPtr->zh_annotator.value().ContainsCharacterSequence(characterSequence))
            cardsWithCharSeq.insert(id);
    }
    fmt::print("relevant cardIds: {}\n", fmt::join(cardsWithCharSeq, ","));

    for (uint cardId : cardsWithCharSeq) {
        auto& cardPtr = cardDB->get().at(cardId);
        cardPtr->zh_annotator.value().SetAnnotationChoices(annotationChoices);
        cardPtr->zh_annotator.value().Reannotate();
        EraseVocabulary(cardId);
        InsertVocabulary(cardPtr->zh_annotator.value().UniqueItems(), cardId);
    }
    getCardNeedsCleanup = true;

    const auto& card = cardDB->get().at(activeCardId);
    return {std::unique_ptr<Card>(card->clone()),
            GetRelevantVocables(activeCardId),
            GetRelevantEase(activeCardId)};
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
    id_cardSR[activeCardId].ViewNow();
}

void VocabularySR::SaveJsonToFile(const std::string_view& fn, const nlohmann::json& js) {
    namespace fs = std::filesystem;
    fs::path fn_metaFile = fs::path(s_path_meta) / fn;
    std::ofstream ofs(fn_metaFile);
    ofs << js.dump(4);
}

void VocabularySR::SaveProgress() {
    auto generateJsonFromMap = [](const auto& map) -> nlohmann::json {
        nlohmann::json jsonMeta = nlohmann::json::object();
        auto& content = jsonMeta[std::string(s_content)];
        content = nlohmann::json::array();

        using sr_t = typename std::decay_t<decltype(map)>::mapped_type;
        ranges::transform(map, std::back_inserter(content), &sr_t::toJson);
        return jsonMeta;
    };
    std::filesystem::create_directory(s_path_meta);

    try {
        // save file for VocableSR --------------------------------------------
        nlohmann::json jsonVocSR = generateJsonFromMap(id_vocableSR);
        SaveJsonToFile(s_fn_metaVocableSR, jsonVocSR);

        // save file for CardSR -----------------------------------------------
        nlohmann::json jsonCardSR = generateJsonFromMap(id_cardSR);
        SaveJsonToFile(s_fn_metaCardSR, jsonCardSR);
    } catch (const std::exception& e) {
        fmt::print("Saving of meta files failed. Error: {}\n", e.what());
    }
}

void VocabularySR::SaveAnnotationChoices() {
    try {
        nlohmann::json array = nlohmann::json::array();
        ranges::transform(annotationChoices,
                          std::back_inserter(array),
                          [](const std::pair<CharacterSequence, Combination>& choice) -> nlohmann::json {
                              return {{"char_seq", choice.first}, {"combination", choice.second}};
                          });
        SaveJsonToFile(s_fn_annotationChoices, array);
    } catch (const std::exception& e) {
        fmt::print("Saving Annotation Choices failed with Error: {}\n", e.what());
    }
}

auto VocabularySR::LoadJsonFromFile(const std::string_view& fn) -> nlohmann::json {
    namespace fs = std::filesystem;
    fs::path fn_metaFile = fs::path(s_path_meta) / fn;
    std::ifstream ifs(fn_metaFile);
    return nlohmann::json::parse(ifs);
}

void VocabularySR::LoadProgress() {
    auto jsonToMap = [](auto& map, const nlohmann::json& jsonMeta) {
        const auto& content = jsonMeta.at(std::string(s_content));
        using sr_t = typename std::decay_t<decltype(map)>::mapped_type;
        ranges::transform(content, std::inserter(map, map.begin()), &sr_t::fromJson);
    };
    try {
        nlohmann::json jsonVocSR = LoadJsonFromFile(s_fn_metaVocableSR);
        jsonToMap(id_vocableSR, jsonVocSR);
        fmt::print("Vocable SR file {} loaded!\n", s_fn_metaVocableSR);
    } catch (const std::exception& e) {
        fmt::print("Vocabulary SR load for {} failed! Exception {}", s_fn_metaVocableSR, e.what());
    }
    try {
        nlohmann::json jsonCardSR = LoadJsonFromFile(s_fn_metaCardSR);
        jsonToMap(id_cardSR, jsonCardSR);
        fmt::print("Card SR file {} loaded!\n", s_fn_metaCardSR);
    } catch (const std::exception& e) {
        fmt::print("Card SR load for {} failed! Exception {}", s_fn_metaCardSR, e.what());
    }

    fmt::print("Vocables studied so far: {}, Cards viewed: {}\n", id_vocableSR.size(), id_cardSR.size());
}

void VocabularySR::LoadAnnotationChoices() {
    try {
        nlohmann::json choicesJson = LoadJsonFromFile(s_fn_annotationChoices);
        ranges::transform(choicesJson,
                          std::inserter(annotationChoices, annotationChoices.begin()),
                          [](const nlohmann::json& choice) -> std::pair<CharacterSequence, Combination> {
                              nlohmann::json char_seqJson = choice["char_seq"];
                              nlohmann::json combinationJson = choice["combination"];
                              CharacterSequence char_seq;
                              Combination combination;
                              ranges::transform(char_seqJson,
                                                std::back_inserter(char_seq),
                                                [](const nlohmann::json& character) -> utl::ItemU8 {
                                                    return {std::string(character)};
                                                });
                              ranges::transform(combinationJson,
                                                std::back_inserter(combination),
                                                [](const nlohmann::json& c) -> int { return c; });
                              return {char_seq, combination};
                          });
    } catch (const std::exception& e) {
        fmt::print("Load of AnnotationChoice file failed, Error: {}\n", e.what());
    }
}

void VocabularySR::GenerateToRepeatWorkload() {
    for (const auto& [id, vocSR] : id_vocableSR) {
        if (vocSR.isToBeRepeatedToday())
            ids_repeatTodayVoc.insert(id);
        if (vocSR.isAgainVocable())
            ids_againVoc.insert(id);
    }
}
