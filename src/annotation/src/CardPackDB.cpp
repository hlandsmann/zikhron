#include "CardPackDB.h"

#include "Card.h" // IWYU pragma: keep
#include "CardPack.h"
#include "Tokenizer.h"
#include "WordDB.h"

#include <misc/Config.h>
#include <misc/Identifier.h>
#include <utils/Memory.h>
#include <utils/format.h>

#include <algorithm>
#include <filesystem>
#include <generator>
#include <iterator>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace ranges = std::ranges;

namespace annotation {
CardPackDB::CardPackDB(std::shared_ptr<zikhron::Config> config,
                       std::shared_ptr<WordDB> _wordDB)
    : wordDB{std::move(_wordDB)}
    , tokenizer{std::make_shared<annotation::Tokenizer>(config, wordDB)}

    , cardPacks{loadCardPacks(config->DatabaseDirectory() / s_packSubdirectory,
                              wordDB,
                              tokenizer)}
    , name_cardPack{setupNameCardPack(cardPacks)}

{
    setupCards();
}

auto CardPackDB::getCards() const -> const std::map<CardId, CardAudio>&
{
    return cards;
}

auto CardPackDB::getTokenizer() const -> std::shared_ptr<annotation::Tokenizer>
{
    return tokenizer;
}

auto CardPackDB::getAnnotationAlternativesForCard(CardId cardId) const -> std::vector<Alternative>
{
    return cards.at(cardId).card->getAlternatives();
}

auto CardPackDB::getCardAtCardId(CardId cardId) const -> const CardAudio&
{
    return cards.at(cardId);
}

auto CardPackDB::getCardPackForCardId(CardId cardId) const -> CardPackPtr
{
    auto card = cards.at(cardId).card;
    auto cardPackId = card->getPackId();
    return cardPacks.at(cardPackId);
}

auto CardPackDB::getCardPack(const std::string& packName) const -> CardPackPtr
{
    return name_cardPack.at(packName);
}

auto CardPackDB::loadCardPacks(const std::filesystem::path& directory,
                               const std::shared_ptr<WordDB>& wordDB,
                               const std::shared_ptr<Tokenizer>& tokenizer) -> std::vector<CardPackPtr>
{
    std::set<std::filesystem::path> spacks;
    ranges::copy_if(std::filesystem::directory_iterator(directory), std::inserter(spacks, spacks.begin()),
                    [](const std::filesystem::path& file) -> bool { return file.extension() == s_spackExtension; });

    std::vector<CardPackPtr> cardPacksResult;

    ranges::transform(spacks, std::back_inserter(cardPacksResult),
                      [&, packCounter = unsigned{}](const std::filesystem::path& spackFile) mutable -> std::shared_ptr<CardPack> {
                          auto packId = static_cast<PackId>(packCounter);
                          packCounter++;
                          return std::make_shared<CardPack>(spackFile, packId, wordDB, tokenizer);
                      });
    return cardPacksResult;
}

auto CardPackDB::setupNameCardPack(const std::vector<CardPackPtr>& cardPacks)
        -> std::map<std::string, CardPackPtr>
{
    std::map<std::string, CardPackPtr> name_cardPack;

    ranges::transform(cardPacks, std::inserter(name_cardPack, name_cardPack.begin()),
                      [](const CardPackPtr& cardPack) -> std::pair<std::string, CardPackPtr> {
                          return {cardPack->getName(), cardPack};
                      });
    return name_cardPack;
}

auto CardPackDB::traverseCards() const -> std::generator<CardAudio>
{
    for (const auto& cardPack : cardPacks) {
        for (const auto& cardAudio : cardPack->getCards()) {
            co_yield cardAudio;
        }
    }
    co_return;
}

void CardPackDB::setupCards()
{
    ranges::transform(traverseCards(), std::inserter(cards, cards.begin()),
                      [cardId = unsigned{}](const CardAudio& cardAudio) mutable -> std::pair<CardId, CardAudio> {
                          cardId++;
                          cardAudio.card->setCardId(static_cast<CardId>(cardId));
                          return {static_cast<CardId>(cardId), cardAudio};
                      });
}
} // namespace annotation
