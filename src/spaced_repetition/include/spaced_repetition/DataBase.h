#pragma once
#include "CardMeta.h"
#include "VocableMeta.h"
#include "VocableProgress.h"
#include "srtypes.h"

#include <annotation/Card.h>
#include <annotation/CardDB.h>
#include <annotation/Ease.h>
#include <annotation/WordDB.h>
#include <dictionary/ZH_Dictionary.h>
#include <misc/Config.h>
#include <misc/Identifier.h>
#include <multimedia/CardAudioGroup.h>
#include <spdlog/spdlog.h>
#include <utils/StringU8.h>
#include <utils/index_map.h>
#include <utils/min_element_val.h>

#include <map>
#include <memory>
#include <nlohmann/json_fwd.hpp>

#include <sys/types.h>

namespace sr {
class DataBase
{
    using CardDB = annotation::CardDB;
    using WordDB = annotation::WordDB;

public:
    using CharacterSequence = annotation::Card::CharacterSequence;

    DataBase(std::shared_ptr<zikhron::Config> config);
    virtual ~DataBase();

    DataBase(const DataBase&) = delete;
    DataBase(DataBase&&) = delete;
    auto operator=(const DataBase&) -> DataBase& = delete;
    auto operator=(DataBase&&) -> DataBase& = delete;
    void save();

    [[nodiscard]] auto Vocables() const -> const utl::index_map<VocableId, VocableMeta>&;
    [[nodiscard]] auto Cards() -> utl::index_map<CardId, CardMeta>&;
    [[nodiscard]] auto getCardDB() const -> std::shared_ptr<CardDB>;
    [[nodiscard]] auto getGroupDB() const -> std::shared_ptr<CardAudioGroupDB>;
    [[nodiscard]] auto getWordDB() const -> std::shared_ptr<WordDB>;

    void setEaseVocable(VocableId, const Ease&);
    void triggerVocable(VocableId, CardId);
    void resetCardsContainingVocable(VocableId vocId);

private:
    [[nodiscard]] auto generateVocableIdProgressMap() const -> std::map<VocableId, VocableProgress>;
    void fillIndexMaps();

    std::shared_ptr<zikhron::Config> config;

    std::shared_ptr<const ZH_Dictionary> zhDictionary;

    std::shared_ptr<CardAudioGroupDB> groupDB;
    std::shared_ptr<annotation::WordDB> wordDB;
    std::shared_ptr<annotation::CardDB> cardDB;
    std::map<VocableId, VocableProgress> progressVocables;

    std::shared_ptr<utl::index_map<VocableId, VocableMeta>> vocables;
    std::shared_ptr<utl::index_map<CardId, CardMeta>> cards;
};
} // namespace sr
