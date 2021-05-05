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
#include <limits>
#include <nlohmann/json.hpp>
#include <ranges>
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

    CalculateCardValues();
    for (const auto& cm : std::span(cardMeta.begin(), std::min(cardMeta.begin() + 32, cardMeta.end()))) {
        std::cout << "Cm id: " << cm->cardId << " value: " << cm->value
                  << " len: " << cm->vocableIds.size() << "\n";
        for (uint vid : cm->vocableIds) {
            std::cout << " - " << id_vocable[vid].front().key;
        }
        std::cout << "\n";
    }
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
            setVocIdThis.begin(), setVocIdThis.end(), good.begin(), good.end(), counting_iterator())
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

void VocabularySR::CalculateCardValues() {
    for (std::shared_ptr<CardMeta>& cm : cardMeta) {
        float quantity_of_vocable_usage = 0;
        for (auto vId : cm->vocableIds) {
            const auto& vocableMeta = id_vocableMeta[vId];
            quantity_of_vocable_usage += vocableMeta.cardIds.size();
        }
        // cm->value = quantity_of_vocable_usage / std::sqrt(static_cast<float>(cm->vocableIds.size()));
        // cm->value = CalculateCardValueSingle(cm->cardId);
    }

    ranges::sort(cardMeta, std::ranges::greater{}, &CardMeta::value);
}

void VocabularySR::InsertVocabulary(const std::set<ZH_Annotator::Item>& cardVocabulary, uint cardId) {
    auto& cardMetaPtr = id_cardMeta[cardId];
    if (cardMetaPtr == nullptr) {
        cardMetaPtr = std::make_shared<CardMeta>();
        cardMeta.push_back(cardMetaPtr);
    }
    CardMeta& cm = *cardMetaPtr;

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

auto VocabularySR::getCard() -> std::pair<std::unique_ptr<Card>, std::vector<ZH_Dictionary::Item>> {
    std::function<float(const std::shared_ptr<CardMeta>&)> calcValue;

    if (ids_repeatTodayVoc.empty()) {
        std::set<uint> studiedVocableIds;
        ranges::copy(id_vocableSR | std::views::keys,
                     std::inserter(studiedVocableIds, studiedVocableIds.begin()));
        calcValue = [&, studiedVocableIds](const std::shared_ptr<CardMeta>& cm) -> float {
            return CalculateCardValueSingleNewVoc(*cm, studiedVocableIds);
        };
    } else {
        calcValue = [&](const std::shared_ptr<CardMeta>& cm) -> float {
            return CalculateCardValueSingle(*cm, ids_repeatTodayVoc);
        };
    }

    const auto& it_cm = ranges::max_element(cardMeta, ranges::less{}, calcValue);
    if (it_cm == cardMeta.end())
        return {nullptr, {}};
    uint cardId = it_cm->get()->cardId;
    return {std::unique_ptr<Card>(cardDB->get().at(cardId)->clone()), GetRelevantVocables(cardId)};
}

auto VocabularySR::GetRelevantVocables(uint cardId) -> std::vector<ZH_Dictionary::Item> {
    std::set<uint> activeVocables;
    const CardMeta& cm = *(id_cardMeta.at(cardId));

    auto vocableActive = [&](uint vocId) -> bool {
        return id_vocableSR.find(vocId) == id_vocableSR.end() ||
               ids_repeatTodayVoc.find(vocId) != ids_repeatTodayVoc.end() ||
               ids_againVoc.find(vocId) != ids_againVoc.end();
    };
    ranges::copy_if(cm.vocableIds, std::inserter(activeVocables, activeVocables.begin()), vocableActive);

    std::vector<ZH_Dictionary::Item> relevantVocables;
    ranges::transform(activeVocables,
                      std::back_inserter(relevantVocables),
                      [&](uint vocId) -> ZH_Dictionary::Item { return id_vocable.at(vocId).front(); });

    std::cout << "CardID: " << cardId;
    std::cout << "size one: " << relevantVocables.size() << " size two: " << cm.vocableIds.size()
              << "\n";

    ids_activeVoc = std::move(activeVocables);
    return relevantVocables;
}

void VocabularySR::setEaseLastCard(Ease ease) {
    using namespace std::literals;
    float easeChange = [ease]() -> float {
        switch (ease) {
        case Ease::easy: return 1.2;
        case Ease::good: return 1.0;
        case Ease::hard: return 0.8;
        default: return 0.;
        }
    }();

    for (uint id : ids_activeVoc) {
        VocableSR& vocableSR = id_vocableSR[id];
        vocableSR = {.vocId = id,
                     .ease_factor = std::clamp(vocableSR.ease_factor * easeChange, 1.3f, 2.5f),
                     .interval_day = vocableSR.interval_day,
                     .last_seen = std::time(nullptr)};

        vocableSR.advanceByEase(ease);

        if (ease == Ease::again)
            ids_againVoc.insert(id);
        ids_repeatTodayVoc.erase(id);
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

        ranges::transform(map | std::views::values,
                          std::back_inserter(content),
                          std::identity{},
                          &std::decay_t<decltype(map)>::mapped_type::toJson);
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
    namespace fs = std::filesystem;
    fs::path fn_metaVocableSR = fs::path(s_path_meta) / s_fn_metaVocableSR;
    nlohmann::json jsonVocSR;
    try {
        std::ifstream ifs(fn_metaVocableSR);
        jsonVocSR = nlohmann::json::parse(ifs);
        auto& content = jsonVocSR.at(std::string(s_content));
        ranges::transform(content,
                          std::inserter(id_vocableSR, id_vocableSR.begin()),
                          [](const nlohmann::json& v) -> std::pair<uint, VocableSR> {
                              VocableSR vocSR = VocableSR::fromJson(v);
                              return {{vocSR.vocId}, {vocSR}};
                          });

    } catch (const std::exception& e) {
        fmt::print("VocabularySR load for {} failed! Exception: {}", fn_metaVocableSR, e.what());
        return;
    }
    fmt::print("Progress loaded from file {}", fn_metaVocableSR);
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
        std::tm vocActiveTime_tm = *std::localtime(&vocSR.last_seen);
        vocActiveTime_tm.tm_mday += vocSR.interval_day;
        std::time_t vocActiveTime = std::mktime(&vocActiveTime_tm);

        if (todayMidnight > vocActiveTime) {
            fmt::print("active: {}\n", std::put_time(&vocActiveTime_tm, "%F %T"));
            ids_repeatTodayVoc.insert(vocId);
        } else {
            fmt::print("next time: {}\n", std::put_time(&vocActiveTime_tm, "%F %T"));
        }
    }
}
