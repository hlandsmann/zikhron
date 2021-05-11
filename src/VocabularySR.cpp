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
            uint vocId = GetNextFreeId();
            id_vocableMeta[vocId] = {.cardIds = {cardId}};
            zhdic_vocableMeta[item.dicItemVec] = vocId;
            id_vocable[vocId] = ZH_dicItemVec{item.dicItemVec};

            cm.vocableIds.insert(vocId);
        }
    }
    cm.cardId = cardId;
}

auto VocabularySR::GetNextFreeId() -> uint {
    if (not id_vocable.empty())
        return id_vocable.rbegin()->first + 1;
    else
        return 0;
}
auto VocabularySR::GetCardRepeatedVoc() -> std::optional<uint> {
    struct intersect_view {
        size_t countIntersect{};
        uint viewCount{};
    };
    std::map<uint, intersect_view> candidates;
    for (const auto& [id, cardMeta] : id_cardMeta) {  //   std::set<uint> test;
        std::vector<uint> out;
        size_t count_union =
            ranges::set_union(id_vocableSR | std::views::keys, cardMeta.vocableIds, counting_iterator{})
                .out.count;
        if (count_union != id_vocableSR.size())
            continue;
        size_t count_intersect = ranges::set_intersection(
                                     ids_repeatTodayVoc, cardMeta.vocableIds, counting_iterator{})
                                     .out.count;
        if (count_intersect == 0)
            continue;

        candidates[id].countIntersect = count_intersect;
        if (const auto& it = id_cardSR.find(id); it != id_cardSR.end())
            candidates[id].viewCount = it->second.viewCount;
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
                   if (a.viewCount == b.viewCount)
                       return a.countIntersect < b.countIntersect;
                   if (std::abs(int(a.countIntersect) - int(b.countIntersect)) <= 1)
                       return a.viewCount < b.viewCount;
                   return a.countIntersect < b.countIntersect;
               },
               &decltype(candidates)::value_type::second)
        ->first;
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
    uint cardId;
    if (auto cardRepeatVoc = GetCardRepeatedVoc(); cardRepeatVoc.has_value()) {
        cardId = cardRepeatVoc.value();
        fmt::print("Get Card with repeated vocables #{}\n", cardId);
    } else if (auto cardNewVoc = GetCardNewVoc(); cardNewVoc.has_value()) {
        cardId = cardNewVoc.value();
        fmt::print("Get new words from Card #{}\n", cardId);
    } else
        return {nullptr, Item_Id_vt{}, Id_Ease_vt{}};

    id_cardSR[cardId].ViewNow();
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

    ids_activeVoc = std::move(activeVocables);
    return relevantVocables;
}

auto VocabularySR::GetRelevantEase(uint cardId) -> Id_Ease_vt {
    std::set<uint> activeVocables = GetActiveVocables(cardId);
    Id_Ease_vt ease;
    ranges::transform(
        activeVocables, std::inserter(ease, ease.begin()), [&](uint vocId) -> Id_Ease_vt::value_type {
            float easeFactor = id_vocableSR.at(vocId).easeFactor;
            if (easeFactor <= 1.31)
                return {vocId, Ease::again};
            if (easeFactor <= 1.6)
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

        if (ease == Ease::again)
            ids_againVoc.insert(id);
        ids_repeatTodayVoc.erase(id);

        fmt::print("Ease of {} is {}\n", id_vocable.at(id).front().key, mapEaseToInt(ease));
    }

    ids_activeVoc.clear();
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
    std::time_t now = std::time(nullptr);
    std::tm todayMidnight_tm = *std::localtime(&now);
    todayMidnight_tm.tm_sec = 0;
    todayMidnight_tm.tm_min = 0;
    todayMidnight_tm.tm_hour = 0;
    todayMidnight_tm.tm_mday += 1;
    std::time_t todayMidnight = std::mktime(&todayMidnight_tm);

    for (auto& [vocId, vocSR] : id_vocableSR) {
        std::tm vocActiveTime_tm = *std::localtime(&vocSR.lastSeen);
        vocActiveTime_tm.tm_mday += vocSR.intervalDay;
        std::time_t vocActiveTime = std::mktime(&vocActiveTime_tm);

        if (todayMidnight > vocActiveTime) {
            fmt::print("active: {}\n", std::put_time(&vocActiveTime_tm, "%F %T"));
            ids_repeatTodayVoc.insert(vocId);
        } else {
            fmt::print("next time: {}\n", std::put_time(&vocActiveTime_tm, "%F %T"));
        }
    }
}
