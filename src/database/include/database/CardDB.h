#pragma once
#include "CardPackDB.h"
#include "CbdFwd.h"
#include "TokenizationChoiceDB.h"
#include "Track.h"
#include "Video.h"
#include "VideoDB.h"
#include "WordDB.h"

#include <annotation/Tokenizer.h>
#include <misc/Config.h>
#include <misc/Identifier.h>

#include <map>
#include <memory>
#include <vector>

namespace database {

class CardDB
{
public:
    CardDB(std::shared_ptr<zikhron::Config> config,
           std::shared_ptr<WordDB> wordDB,
           std::shared_ptr<CardPackDB> cardPackDB,
           std::shared_ptr<VideoDB> videoDB,
           std::shared_ptr<TokenizationChoiceDB> tokenizationChoiceDB);

    void save();
    [[nodiscard]] auto getCards() const -> const std::map<CardId, CardPtr>&;
    [[nodiscard]] auto getAnnotationAlternativesForCard(CardId cardId) const
            -> std::vector<annotation::Alternative>;
    [[nodiscard]] auto getCardPackDB() const -> std::shared_ptr<CardPackDB>;
    [[nodiscard]] auto getVideoDB() const -> std::shared_ptr<VideoDB>;
    [[nodiscard]] auto getTrackFromCardId(CardId cardId) const -> Track;
    [[nodiscard]] auto getTrackFromVideo(const VideoPtr& video) const -> Track;

    void addCard(const CardPtr& card);
    void eraseCard(CardId cardId);

private:
    std::shared_ptr<zikhron::Config> config;
    std::shared_ptr<WordDB> wordDB;
    std::shared_ptr<CardPackDB> cardPackDB;
    std::shared_ptr<VideoDB> videoDB;
    std::shared_ptr<TokenizationChoiceDB> tokenizationChoiceDB;

    std::map<CardId, CardPtr> cards;
};

} // namespace database
