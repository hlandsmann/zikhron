#pragma once
#include "CardMeta.h"
#include "VocableMeta.h"
#include "srtypes.h"

#include <annotation/Ease.h>
#include <database/Card.h>
#include <database/CardDB.h>
#include <database/CardPackDB.h>
#include <database/CbdFwd.h>
#include <database/TokenizationChoiceDB.h>
#include <database/VideoDB.h>
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
#include <optional>

#include <sys/types.h>

namespace sr {
class DataBase
{
    using WordDB = database::WordDB;
    using CardDB = database::CardDB;
    using CardPackDB = database::CardPackDB;
    using VideoDB = database::VideoDB;
    using TokenizationChoiceDB = database::TokenizationChoiceDB;

public:
    using CharacterSequence = database::Card::CharacterSequence;

    DataBase(std::shared_ptr<zikhron::Config> config,
             std::shared_ptr<WordDB> wordDB,
             std::shared_ptr<CardDB> cardDB,
             std::shared_ptr<TokenizationChoiceDB> tokenizationChoiceDB);
    virtual ~DataBase();

    DataBase(const DataBase&) = delete;
    DataBase(DataBase&&) = delete;
    auto operator=(const DataBase&) -> DataBase& = delete;
    auto operator=(DataBase&&) -> DataBase& = delete;
    void save();

    [[nodiscard]] auto Vocables() const -> const utl::index_map<VocableId, VocableMeta>&;
    [[nodiscard]] auto MetaCards() -> const utl::index_map<CardId, CardMeta>&;
    [[nodiscard]] auto getTokenizationChoiceDB() const -> std::shared_ptr<database::TokenizationChoiceDB>;
    [[nodiscard]] auto getCardDB() const -> std::shared_ptr<CardDB>;
    [[nodiscard]] auto getWordDB() const -> std::shared_ptr<WordDB>;

    [[nodiscard]] auto getCardMeta(const database::CardPtr& card) -> const CardMeta&;
    [[nodiscard]] auto getCardMeta(CardId cardId) -> const CardMeta&;
    void reloadCard(const database::CardPtr& card);

    void setEaseVocable(VocableId, const Ease&);
    void triggerVocable(VocableId, CardId);
    void resetCardsContainingVocable(VocableId vocId);

    [[nodiscard]] auto cardExists(CardId cardId) const -> bool;
    void addCard(CardId cardId);
    void removeCard(CardId cardId);

private:
    [[nodiscard]] auto generateVocableIdProgressMap() const -> std::map<VocableId, VocableProgress>;
    void fillIndexMaps();

    std::shared_ptr<zikhron::Config> config;

    std::shared_ptr<WordDB> wordDB;
    std::shared_ptr<CardDB> cardDB;
    std::shared_ptr<TokenizationChoiceDB> tokenizationChoiceDB;
    std::map<VocableId, VocableProgress> progressVocables;

    std::shared_ptr<utl::index_map<VocableId, VocableMeta>> vocables;
    utl::index_map<CardId, CardMeta> metaCards;

    std::shared_ptr<CardMeta> temporaryCardMeta;
};
} // namespace sr
