#pragma once
#include "CardMeta.h"
#include "VocableMeta.h"
#include "srtypes.h"

#include <annotation/Ease.h>
#include <database/Card.h>
#include <database/CardPackDB.h>
#include <database/CbdFwd.h>
#include <database/TokenizationChoiceDB.h>
#include <database/VideoPackDB.h>
#include <database/VocableProgress.h>
#include <database/WordDB.h>
#include <dictionary/ZH_Dictionary.h>
#include <misc/Config.h>
#include <misc/Identifier.h>
#include <spdlog/spdlog.h>
#include <utils/StringU8.h>
#include <utils/index_map.h>
#include <utils/min_element_val.h>

#include <map>
#include <memory>

#include <sys/types.h>

namespace sr {
class DataBase
{
    using WordDB = database::WordDB;
    using CardPackDB = database::CardPackDB;
    using VideoPackDB = database::VideoPackDB;
    using TokenizationChoiceDB = database::TokenizationChoiceDB;

public:
    using CharacterSequence = database::Card::CharacterSequence;

    DataBase(std::shared_ptr<zikhron::Config> config);
    virtual ~DataBase();

    DataBase(const DataBase&) = delete;
    DataBase(DataBase&&) = delete;
    auto operator=(const DataBase&) -> DataBase& = delete;
    auto operator=(DataBase&&) -> DataBase& = delete;
    void save();

    [[nodiscard]] auto Vocables() const -> const utl::index_map<VocableId, VocableMeta>&;
    [[nodiscard]] auto Cards() -> utl::index_map<CardId, CardMeta>&;
    [[nodiscard]] auto getCardPackDB() const -> std::shared_ptr<CardPackDB>;
    [[nodiscard]] auto getVideoPackDB() const -> std::shared_ptr<VideoPackDB>;
    [[nodiscard]] auto getTokenizationChoiceDB() const -> std::shared_ptr<database::TokenizationChoiceDB>;
    [[nodiscard]] auto getWordDB() const -> std::shared_ptr<WordDB>;
    void reloadCard(const database::CardPtr& card);

    void setEaseVocable(VocableId, const Ease&);
    void triggerVocable(VocableId, CardId);
    void resetCardsContainingVocable(VocableId vocId);

private:
    [[nodiscard]] auto generateVocableIdProgressMap() const -> std::map<VocableId, VocableProgress>;
    void fillIndexMaps();

    std::shared_ptr<zikhron::Config> config;

    std::shared_ptr<const ZH_Dictionary> zhDictionary;

    std::shared_ptr<WordDB> wordDB;
    std::shared_ptr<CardPackDB> cardPackDB;
    std::shared_ptr<VideoPackDB> videoPackDB;
    std::shared_ptr<TokenizationChoiceDB> tokenizationChoiceDB;
    std::map<VocableId, VocableProgress> progressVocables;

    std::shared_ptr<utl::index_map<VocableId, VocableMeta>> vocables;
    std::shared_ptr<utl::index_map<CardId, CardMeta>> cards;
};
} // namespace sr
