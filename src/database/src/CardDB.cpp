#include "CardDB.h"

#include "CardPack.h"
#include "CardPackDB.h"
#include "CbdFwd.h"
#include "TokenizationChoiceDB.h"
#include "Track.h"
#include "VideoDB.h"
#include "WordDB.h"

#include <annotation/Tokenizer.h>
#include <misc/Config.h>
#include <misc/Identifier.h>

#include <algorithm>
#include <iterator>
#include <map>
#include <memory>
#include <utility>
#include <vector>

namespace ranges = std::ranges;

namespace database {

CardDB::CardDB(std::shared_ptr<zikhron::Config> _config,
               std::shared_ptr<WordDB> _wordDB,
               std::shared_ptr<CardPackDB> _cardPackDB,
               std::shared_ptr<VideoDB> _videoDB,
               std::shared_ptr<TokenizationChoiceDB> _tokenizationChoiceDB)
    : config{std::move(_config)}
    , wordDB{std::move(_wordDB)}
    , cardPackDB{std::move(_cardPackDB)}
    , videoDB{std::move(_videoDB)}
    , tokenizationChoiceDB{std::move(_tokenizationChoiceDB)}
{
    ranges::transform(cardPackDB->getCardAudio(),
                      std::inserter(cards, cards.begin()),
                      [](const std::pair<CardId, CardAudio>& id_cardAudio) -> std::pair<CardId, CardPtr> {
                          return {id_cardAudio.first, id_cardAudio.second.card};
                      });
}

void CardDB::save()
{
    videoDB->save();
}

auto CardDB::getCards() const -> const std::map<CardId, CardPtr>&
{
    return cards;
}

auto CardDB::getAnnotationAlternativesForCard(CardId cardId) const -> std::vector<annotation::Alternative>
{
    return cards.at(cardId)->getAlternatives();
}

auto CardDB::getCardPackDB() const -> std::shared_ptr<CardPackDB>
{
    return cardPackDB;
}

auto CardDB::getVideoDB() const -> std::shared_ptr<VideoDB>
{
    return videoDB;
}

auto CardDB::getTrackFromCardId(CardId cardId) const -> Track
{
    auto cardPack = cardPackDB->getCardPackForCardId(cardId);
    auto card = cards.at(cardId);
    return {TrackMedia{cardPack}, card};
}

} // namespace database
