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

auto VocabularySR::addAnnotation(const std::vector<int>& combination,
                                 const std::vector<utl::CharU8>& characterSequence) -> CardInformation {
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
        InsertVocabulary(cardId);
    }
    getCardNeedsCleanup = true;

    const auto& card = cardDB->get().at(activeCardId);
    return {std::unique_ptr<Card>(card->clone()),
            GetVocableIdsInOrder(activeCardId),
            GetRelevantEase(activeCardId)};
}

auto VocabularySR::addVocableChoice(uint vocId, uint vocIdOldChoice, uint vocIdNewChoice)
    -> CardInformation {
    if (vocIdOldChoice == vocIdNewChoice) {
        const auto& card = cardDB->get().at(activeCardId);
        return {std::unique_ptr<Card>(card->clone()),
                GetVocableIdsInOrder(activeCardId),
                GetRelevantEase(activeCardId)};
    }

    if (vocId != vocIdNewChoice)
        id_id_vocableChoices[vocId] = vocIdNewChoice;
    else
        id_id_vocableChoices.erase(vocId);
    auto& vocableMeta = id_vocableMeta.at(vocIdOldChoice);
    for (uint cardId : vocableMeta.cardIds) {
        auto& cardMeta = id_cardMeta.at(cardId);
        cardMeta.vocableIds.erase(vocIdOldChoice);
        cardMeta.vocableIds.insert(vocIdNewChoice);
    }
    id_vocableMeta[vocIdNewChoice] = id_vocableMeta.at(vocIdOldChoice);
    id_vocableSR[vocIdNewChoice] = id_vocableSR.at(vocIdOldChoice);
    id_vocable[vocIdNewChoice] = id_vocable[vocIdOldChoice];
    id_vocable.erase(vocIdOldChoice);
    id_vocableMeta.erase(vocIdOldChoice);
    id_vocableSR.erase(vocIdOldChoice);

    ids_repeatTodayVoc.erase(vocIdOldChoice);
    ids_repeatTodayVoc.insert(vocIdNewChoice);
    ids_againVoc.erase(vocIdOldChoice);
    ids_againVoc.insert(vocIdNewChoice);
    ids_nowVoc.erase(vocIdOldChoice);
    ids_nowVoc.insert(vocIdNewChoice);

    const auto& card = cardDB->get().at(activeCardId);
    return {std::unique_ptr<Card>(card->clone()),
            GetVocableIdsInOrder(activeCardId),
            GetRelevantEase(activeCardId)};
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

void VocabularySR::SaveProgress() const {
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
    }
    catch (const std::exception& e) {
        fmt::print("Saving of meta files failed. Error: {}\n", e.what());
    }
}

void VocabularySR::SaveAnnotationChoices() const {
    try {
        nlohmann::json array = nlohmann::json::array();
        ranges::transform(annotationChoices,
                          std::back_inserter(array),
                          [](const std::pair<CharacterSequence, Combination>& choice) -> nlohmann::json {
                              return {{"char_seq", choice.first}, {"combination", choice.second}};
                          });
        SaveJsonToFile(s_fn_annotationChoices, array);
    }
    catch (const std::exception& e) {
        fmt::print("Saving annotation choices failed with Error: {}\n", e.what());
    }
}

void VocabularySR::SaveVocableChoices() const {
try {
        nlohmann::json array = nlohmann::json::array();
        ranges::transform(id_id_vocableChoices,
                          std::back_inserter(array),
                          [](const std::pair<uint, uint>& choice) -> nlohmann::json {
                              return {{"id", choice.first}, {"map_id", choice.second}};
                          });
        SaveJsonToFile(s_fn_vocableChoices, array);
    }
    catch (const std::exception& e) {
        fmt::print("Saving vocable choices failed with Error: {}\n", e.what());
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
    }
    catch (const std::exception& e) {
        fmt::print("Vocabulary SR load for {} failed! Exception {}", s_fn_metaVocableSR, e.what());
    }
    try {
        nlohmann::json jsonCardSR = LoadJsonFromFile(s_fn_metaCardSR);
        jsonToMap(id_cardSR, jsonCardSR);
        fmt::print("Card SR file {} loaded!\n", s_fn_metaCardSR);
    }
    catch (const std::exception& e) {
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
                                                [](const nlohmann::json& character) -> utl::CharU8 {
                                                    return {std::string(character)};
                                                });
                              ranges::transform(combinationJson,
                                                std::back_inserter(combination),
                                                [](const nlohmann::json& c) -> int { return c; });
                              return {char_seq, combination};
                          });
    }
    catch (const std::exception& e) {
        fmt::print("Load of AnnotationChoice file failed, Error: {}\n", e.what());
    }
}

void VocabularySR::LoadVocableChoices() {
    try {
        nlohmann::json choicesJson = LoadJsonFromFile(s_fn_vocableChoices);
        ranges::transform(choicesJson,
                          std::inserter(id_id_vocableChoices, id_id_vocableChoices.begin()),
                          [](const nlohmann::json& choice) -> std::pair<uint, uint> {
                              nlohmann::json uint_id = choice["id"];
                              nlohmann::json uint_map_id = choice["map_id"];

                              return {uint_id, uint_map_id};
                          });
    }
    catch (const std::exception& e) {
        fmt::print("Load of vocable choice file failed, Error: {}\n", e.what());
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
