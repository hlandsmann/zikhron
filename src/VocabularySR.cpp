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
#include <iomanip>
#include <iostream>
#include <limits>
#include <nlohmann/json.hpp>
#include <ranges>
namespace ranges = std::ranges;

VocabularySR::~VocabularySR() {}
VocabularySR::VocabularySR(CardDB&& _cardDB, std::shared_ptr<ZH_Dictionary> _zh_dictionary)
    : cardDB(std::make_shared<CardDB>(std::move(_cardDB))), zh_dictionary(_zh_dictionary) {
    std::set<ZH_Annotator::Item> myDic;

    GenerateFromCards();
    LoadProgress();
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

auto VocabularySR::CalculateCardValueSingle(uint cardId) -> float {
    struct counting_iterator {
        size_t count;
        counting_iterator& operator++() {
            ++count;
            return *this;
        }

        struct black_hole {
            void operator=(uint) {}
        };
        black_hole operator*() { return black_hole(); }
    };

    std::set<uint> cardIdsSharedVoc;
    int count = 0;

    for (uint vocId : id_cardMeta.at(cardId)->vocableIds) {
        const VocableMeta& vm = id_vocableMeta.at(vocId);
        ranges::copy(vm.cardIds, std::inserter(cardIdsSharedVoc, cardIdsSharedVoc.begin()));
    }

    const auto& setVocIdThis = id_cardMeta.at(cardId)->vocableIds;
    for (uint cId : cardIdsSharedVoc) {
        const auto& setVocOther = id_cardMeta.at(cId)->vocableIds;
        count += pow(std::set_intersection(setVocIdThis.begin(),
                                           setVocIdThis.end(),
                                           setVocOther.begin(),
                                           setVocOther.end(),
                                           counting_iterator())
                         .count,
                     2);
    }
    if (cardIdsSharedVoc.empty())
        return 1.;

    return float(count) / float(setVocIdThis.size());
}

void VocabularySR::CalculateCardValues() {
    for (std::shared_ptr<CardMeta>& cm : cardMeta) {
        float quantity_of_vocable_usage = 0;
        for (auto vId : cm->vocableIds) {
            const auto& vocableMeta = id_vocableMeta[vId];
            quantity_of_vocable_usage += vocableMeta.cardIds.size();
        }
        // cm->value = quantity_of_vocable_usage / std::sqrt(static_cast<float>(cm->vocableIds.size()));
        cm->value = CalculateCardValueSingle(cm->cardId);
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
    if (cardMeta.empty())
        return {nullptr, {}};
    uint cardId = cardMeta.front()->cardId;

    return {std::unique_ptr<Card>(cardDB->get().at(cardId)->clone()), GetRelevantVocables(cardId)};
}

auto VocabularySR::GetRelevantVocables(uint cardId) -> std::vector<ZH_Dictionary::Item> {
    std::set<uint> activeVocables;
    const CardMeta& cm = *(id_cardMeta.at(cardId));

    auto vocableActive = [&](uint vocId) -> bool {
        return id_vocableSR.find(vocId) == id_vocableSR.end();
    };
    ranges::copy_if(cm.vocableIds, std::inserter(activeVocables, activeVocables.begin()), vocableActive);

    std::vector<ZH_Dictionary::Item> relevantVocables;
    ranges::transform(activeVocables,
                      std::back_inserter(relevantVocables),
                      [&](uint vocId) -> ZH_Dictionary::Item { return id_vocable.at(vocId).front(); });

    std::cout << "CardID: " << cardId;
    std::cout << "size one: " << relevantVocables.size() << " size two: " << cm.vocableIds.size()
              << "\n";

    ids_activeVocables = std::move(activeVocables);
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

    for (uint id : ids_activeVocables) {
        auto& vocableSR = id_vocableSR[id];
        vocableSR.ease_factor = std::clamp(vocableSR.ease_factor * easeChange, 1.3f, 2.5f);
        vocableSR.last_seen = std::time(nullptr);

        vocableSR.advanceByEase(ease);
    }
    try {
        SaveProgress();
    } catch (const std::exception& e) { std::cout << e.what() << "\n"; }
}

void VocabularySR::SaveProgress() {
    namespace fs = std::filesystem;
    nlohmann::json jsonVocSR = nlohmann::json::object();
    auto& content = jsonVocSR[std::string(s_content)];
    content = nlohmann::json::array();
    ranges::transform(
        id_vocableSR, std::back_inserter(content), [](std::pair<uint, VocableSR> id_vocSR) {
            nlohmann::json v;
            v[std::string(VocableSR::s_id)] = id_vocSR.first;
            v[std::string(VocableSR::s_ease_factor)] = id_vocSR.second.ease_factor;
            v[std::string(VocableSR::s_interval_day)] = id_vocSR.second.interval_day;
            v[std::string(VocableSR::s_last_seen)] = fmt::format(
                "{}", std::put_time(std::localtime(&id_vocSR.second.last_seen), "%F %T"));
            return v;
        });

    fmt::print("json: {}", jsonVocSR.dump(4));
    fs::create_directory(s_path_meta);
    fs::path fn_metaVocableSR = fs::path(s_path_meta) / s_fn_metaVocableSR;
    std::ofstream ofs(fn_metaVocableSR);
    ofs << jsonVocSR.dump(4);

    fmt::print("{}\n", fn_metaVocableSR);
}

void VocabularySR::LoadProgress() {
    namespace fs = std::filesystem;
    fs::path fn_metaVocableSR = fs::path(s_path_meta) / s_fn_metaVocableSR;
    nlohmann::json jsonVocSR;
    try {
        std::ifstream ifs(fn_metaVocableSR);
        jsonVocSR = nlohmann::json::parse(ifs);
        auto& content = jsonVocSR.at(std::string(s_content));
        ranges::transform(
            content,
            std::inserter(id_vocableSR, id_vocableSR.begin()),
            [](const nlohmann::json& v) -> std::pair<uint, VocableSR> {
                uint id = v.at(std::string(VocableSR::s_id));
                VocableSR vocSR;
                vocSR.ease_factor = v.at(std::string(VocableSR::s_ease_factor));
                vocSR.interval_day = v.at(std::string(VocableSR::s_interval_day));
                std::tm last_seen;
                std::stringstream ss(std::string(v.at(std::string(VocableSR::s_last_seen))));
                ss >> std::get_time(&last_seen, "%Y-%m-%d %H:%M:%S");
                vocSR.last_seen = std::mktime(&last_seen);
                return {{id}, {vocSR}};
            });

    } catch (const std::exception& e) {
        fmt::print("VocabularySR load for {} failed! Exception: {}", fn_metaVocableSR, e.what());
        return;
    }
    fmt::print("Progress loaded from file {}", fn_metaVocableSR);
}

void VocableSR::advanceByEase(Ease ease) {
    if (ease == Ease::again) {
        interval_day = 10. / (60. * 24.); /* approximately 10 minutes */
    } else {
        interval_day = std::max(1.f, interval_day * ease_factor);
    }
}
