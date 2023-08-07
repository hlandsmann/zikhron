#include <DataBase_deprecated.h>
#include <annotation/Markup.h>
#include <annotation/ZH_Annotator.h>
#include <spdlog/spdlog.h>
#include <utils/counting_iterator.h>
#include <algorithm>
#include <boost/range/combine.hpp>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <ranges>
namespace ranges = std::ranges;

SR_DataBase::SR_DataBase(const std::shared_ptr<CardDB>& cardDB_in,
                         const std::shared_ptr<ZH_Dictionary>& zh_dictionary_in)
    : cardDB(cardDB_in), zh_dictionary(zh_dictionary_in) {
    LoadAnnotationChoices();
    LoadVocableChoices();
    GenerateFromCards();
    LoadProgress();
    GenerateToRepeatWorkload();
    CleanUpVocables(std::set<uint>());
}

SR_DataBase::~SR_DataBase() {
    try {
        SaveVocableChoices();
        SaveAnnotationChoices();
        SaveProgress();
    }
    catch (const std::exception& e) {
        spdlog::error(e.what());
    }
}

auto SR_DataBase::LoadJsonFromFile(const std::string_view& fn) -> nlohmann::json {
    namespace fs = std::filesystem;
    fs::path fn_metaFile = fs::path(s_path_meta) / fn;
    std::ifstream ifs(fn_metaFile);
    return nlohmann::json::parse(ifs);
}

void SR_DataBase::SaveJsonToFile(const std::string_view& fn, const nlohmann::json& js) {
    namespace fs = std::filesystem;
    fs::path fn_metaFile = fs::path(s_path_meta) / fn;
    std::ofstream ofs(fn_metaFile);
    ofs << js.dump(4);
}

void SR_DataBase::LoadAnnotationChoices() {
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
        spdlog::error("Load of AnnotationChoice file failed, Error: {}", e.what());
    }
}

void SR_DataBase::SaveAnnotationChoices() const {
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
        spdlog::error("Saving annotation choices failed with Error: {}", e.what());
    }
}

void SR_DataBase::LoadVocableChoices() {
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
        spdlog::error("Load of vocable choice file failed, Error: {}", e.what());
    }
}

void SR_DataBase::SaveVocableChoices() const {
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
        spdlog::error("Saving vocable choices failed with Error: {}", e.what());
    }
}

void SR_DataBase::GenerateFromCards() {
    const std::map<uint, CardDB::CardPtr>& cards = cardDB->get();

    for (const auto& [cardId, card] : cards) {
        InsertVocabularyOfCard(cardId, card);
    }
    std::set<std::string> allCharacters;

    for (const auto& vocable_key : zhdic_vocableMeta | std::views::keys) {
        if (vocable_key.empty())
            continue;

        const auto word = utl::StringU8(vocable_key);
        allCharacters.insert(word.cbegin(), word.cend());
    }

    spdlog::info("Characters: {}", fmt::join(allCharacters, ""));
    spdlog::info("Count of Characters: {}", allCharacters.size());
    spdlog::info("VocableSize: {}", zhdic_vocableMeta.size());
}

void SR_DataBase::EraseVocabularyOfCard(uint cardId) {
    CardMetaDeprecated& cm = id_cardMeta.at(cardId);
    for (uint vocId : cm.vocableIds) {
        auto& vocable = id_vocableMeta.at(vocId);
        vocable.cardIds.erase(cardId);
    }
    cm.vocableIds.clear();
}

void SR_DataBase::InsertVocabularyOfCard(uint cardId, const CardDB::CardPtr& card) {
    // utl::StringU8 card_text = markup::Paragraph::textFromCard(*card);
    card->createAnnotator(zh_dictionary, annotationChoices);

    CardMetaDeprecated& cm = id_cardMeta[cardId];
    const CardDB::CardPtr& cardPtr = cardDB->get().at(cardId);
    // if( not cardPtr.has_value()) return ;
    const ZH_Annotator& annotator = cardPtr->getAnnotator();

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
        const auto& dicEntry = zh_dictionary->EntryFromPosition(vocId, CharacterSetType::Simplified);
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

void SR_DataBase::SetEase(uint vocId, Ease ease) {
    VocableProgress& vocableSR = id_vocableSR[vocId];

    vocableSR.advanceByEase(ease);
    ids_nowVoc.erase(vocId);

    if (ease.ease == EaseVal::again)
        ids_againVoc.insert(vocId);
    else
        ids_againVoc.erase(vocId);

    ids_repeatTodayVoc.erase(vocId);
    spdlog::debug("Ease of {} is {}, intervalDay {:.2f}, easeFactor {:.2f}",
                  id_vocable.at(vocId).front().key,
                  mapEaseToUint(ease.ease),
                  vocableSR.IntervalDay(),
                  vocableSR.EaseFactor());
}

void SR_DataBase::ViewCard(uint cardId) { id_cardSR[cardId].ViewNow(); }

void SR_DataBase::AdvanceIndirectlySeenVocables(uint cardId) {
    std::vector<std::string> advancedVocables;
    std::vector<std::string> unchangedVocables;
    for (uint vocId : id_cardMeta.at(cardId).vocableIds)
        if (auto it = id_vocableSR.find(vocId); it != id_vocableSR.end()) {
            if (it->second.advanceIndirectly())
                advancedVocables.push_back(id_vocable.at(it->first).front().key);
            else
                unchangedVocables.push_back(id_vocable.at(it->first).front().key);
        }

    spdlog::info("Advancing indirectly: {}", fmt::join(advancedVocables, ", "));
    spdlog::debug("Unchanged are: {}", fmt::join(unchangedVocables, ", "));
}

void SR_DataBase::AdvanceFailedVocables() {
    std::set<uint> toRepeatVoc;
    ranges::copy_if(ids_againVoc, std::inserter(toRepeatVoc, toRepeatVoc.begin()), [&](uint id) {
        return id_vocableSR.at(id).pauseTimeOver();
    });
    for (uint id : toRepeatVoc) {
        ids_againVoc.erase(id);
        ids_repeatTodayVoc.insert(id);
        ids_nowVoc.insert(id);
        spdlog::debug("Wait time over for {}", id_vocable.at(id).front().key);
    }
}

auto SR_DataBase::GetVocableIdsInOrder(uint cardId) const -> std::vector<uint> {
    const CardDB::CardPtr& cardPtr = cardDB->get().at(cardId);
    const ZH_Annotator& annotator = cardPtr->getAnnotator();
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

void SR_DataBase::LoadProgress() {
    auto jsonToMap = [](auto& map, const nlohmann::json& jsonMeta) {
        const auto& content = jsonMeta.at(std::string(s_content));
        using sr_t = typename std::decay_t<decltype(map)>::mapped_type;
        ranges::transform(content, std::inserter(map, map.begin()), &sr_t::fromJson);
    };
    try {
        nlohmann::json jsonVocSR = LoadJsonFromFile(s_fn_metaVocableSR);
        jsonToMap(id_vocableSR, jsonVocSR);
        spdlog::debug("Vocable SR file {} loaded!", s_fn_metaVocableSR);
    }
    catch (const std::exception& e) {
        spdlog::error("Vocabulary SR load for {} failed! Exception {}", s_fn_metaVocableSR, e.what());
    }
    try {
        nlohmann::json jsonCardSR = LoadJsonFromFile(s_fn_metaCardSR);
        jsonToMap(id_cardSR, jsonCardSR);
        spdlog::debug("Card SR file {} loaded!", s_fn_metaCardSR);
    }
    catch (const std::exception& e) {
        spdlog::error("Card SR load for {} failed! Exception {}", s_fn_metaCardSR, e.what());
    }

    spdlog::info("Vocables studied so far: {}, Cards viewed: {}", id_vocableSR.size(), id_cardSR.size());
}

void SR_DataBase::SaveProgress() const {
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
        spdlog::error("Saving of meta files failed. Error: {}", e.what());
    }
}

void SR_DataBase::GenerateToRepeatWorkload() {
    for (const auto& [id, vocSR] : id_vocableSR) {
        if (vocSR.isToBeRepeatedToday())
            ids_repeatTodayVoc.insert(id);
        if (vocSR.isAgainVocable())
            ids_againVoc.insert(id);
    }
}

void SR_DataBase::CleanUpVocables(std::set<uint> ignoreVocableIds) {
    std::set<uint> badVocableIds;
    std::set<uint> activeVocableIds;

    ranges::copy(id_vocableSR | std::views::keys,
                 std::inserter(ignoreVocableIds, ignoreVocableIds.begin()));

    auto seenCards = id_cardMeta |
                     std::views::filter([&ignoreVocableIds](std::pair<uint, CardMetaDeprecated> id_cm) {
                         return ranges::set_difference(
                                    id_cm.second.vocableIds, ignoreVocableIds, utl::counting_iterator{})
                                    .out.count == 0;
                     });
    for (const auto& [id, card] : seenCards) {
        activeVocableIds.insert(card.vocableIds.begin(), card.vocableIds.end());
    }

    ranges::set_difference(id_vocableSR | std::views::keys,
                           activeVocableIds,
                           std::inserter(badVocableIds, badVocableIds.begin()));
    for (uint id : badVocableIds) {
        spdlog::warn("Erasing id: {} - key: {}",
                     id,
                     id_vocable.find(id) != id_vocable.end() ? id_vocable[id].front().key : "");
        id_vocableSR.erase(id);
        ids_repeatTodayVoc.erase(id);
        ids_againVoc.erase(id);
        ids_nowVoc.erase(id);
    }
}

void SR_DataBase::AddAnnotation(const ZH_Annotator::Combination& combination,
                                const std::vector<utl::CharU8>& characterSequence,
                                uint activeCardId) {
    annotationChoices[characterSequence] = combination;

    std::set<uint> cardsWithCharSeq;

    for (const auto& [id, cardPtr] : cardDB->get()) {
        if (cardPtr->getAnnotator().ContainsCharacterSequence(characterSequence))
            cardsWithCharSeq.insert(id);
    }
    spdlog::debug("relevant cardIds: {}", fmt::join(cardsWithCharSeq, ","));

    for (uint cardId : cardsWithCharSeq) {
        auto& cardPtr = cardDB->get().at(cardId);
        cardPtr->getAnnotator().SetAnnotationChoices(annotationChoices);
        cardPtr->getAnnotator().Reannotate();
        EraseVocabularyOfCard(cardId);
        InsertVocabularyOfCard(cardId, cardPtr);
    }
    CleanUpVocables(id_cardMeta.at(activeCardId).vocableIds);
}

void SR_DataBase::AddVocableChoice(uint vocId, uint vocIdOldChoice, uint vocIdNewChoice) {
    if (vocIdOldChoice == vocIdNewChoice) {
        return;
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
    id_vocableMeta[vocIdNewChoice] = id_vocableMeta[vocIdOldChoice];
    id_vocableSR[vocIdNewChoice] = id_vocableSR[vocIdOldChoice];
    id_vocable[vocIdNewChoice] = id_vocable[vocIdOldChoice];
    id_vocable.erase(vocIdOldChoice);
    id_vocableMeta.erase(vocIdOldChoice);
    id_vocableSR.erase(vocIdOldChoice);

    if (ids_repeatTodayVoc.contains(vocIdOldChoice)) {
        ids_repeatTodayVoc.erase(vocIdOldChoice);
        ids_repeatTodayVoc.insert(vocIdNewChoice);
    }
    if (ids_againVoc.contains(vocIdOldChoice)) {
        ids_againVoc.erase(vocIdOldChoice);
        ids_againVoc.insert(vocIdNewChoice);
    }
    if (ids_nowVoc.contains(vocIdOldChoice)) {
        ids_nowVoc.erase(vocIdOldChoice);
        ids_nowVoc.insert(vocIdNewChoice);
    }
}
