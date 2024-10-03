#include "CardPackDB.h"

#include "Card.h" // IWYU pragma: keep
#include "CardPack.h"
#include "IdGenerator.h"

#include <annotation/Tokenizer.h>
#include <database/WordDB.h>
#include <misc/Config.h>
#include <misc/Identifier.h>
#include <misc/Language.h>
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

namespace database {
CardPackDB::CardPackDB(std::shared_ptr<zikhron::Config> config,
                       Language language,
                       std::shared_ptr<CardIdGenerator> _cardIdGenerator,
                       std::shared_ptr<PackIdGenerator> _packIdGenerator,
                       std::shared_ptr<database::WordDB> _wordDB,
                       std::shared_ptr<annotation::Tokenizer> _tokenizer)
    : cardIdGenerator{std::move(_cardIdGenerator)}
    , packIdGenerator{std::move(_packIdGenerator)}
    , wordDB{std::move(_wordDB)}
    , tokenizer{std::move(_tokenizer)}
    , idToCardPacks{loadCardPacks(config->DatabaseDirectory() / languageToPackDirectory.at(language),
                                  cardIdGenerator,
                                  packIdGenerator,
                                  wordDB,
                                  tokenizer)}
    , nameToCardPacks{setNameToCardPacks(idToCardPacks)}
{
    setupCards();
}

auto CardPackDB::getCardAudio() const -> const std::map<CardId, CardAudio>&
{
    return cards;
}

auto CardPackDB::getTokenizer() const -> std::shared_ptr<annotation::Tokenizer>
{
    return tokenizer;
}

auto CardPackDB::getCardAtCardId(CardId cardId) const -> const CardAudio&
{
    return cards.at(cardId);
}

auto CardPackDB::getCardPackForCardId(CardId cardId) const -> CardPackPtr
{
    auto card = cards.at(cardId).card;
    auto packId = card->getPackId();
    return idToCardPacks.at(packId);
}

auto CardPackDB::getCardPack(const std::string& packName) const -> CardPackPtr
{
    return nameToCardPacks.at(packName);
}

auto CardPackDB::getCardPack(PackId packId) const -> CardPackPtr
{
    return idToCardPacks.at(packId);
}

auto CardPackDB::loadCardPacks(const std::filesystem::path& directory,
                               std::shared_ptr<CardIdGenerator> cardIdGenerator,
                               std::shared_ptr<PackIdGenerator> packIdGenerator,
                               const std::shared_ptr<database::WordDB>& wordDB,
                               const std::shared_ptr<annotation::Tokenizer>& tokenizer) -> std::map<PackId, CardPackPtr>
{
    std::set<std::filesystem::path> spacks;
    ranges::copy_if(std::filesystem::directory_iterator(directory), std::inserter(spacks, spacks.begin()),
                    [](const std::filesystem::path& file) -> bool { return file.extension() == s_spackExtension; });

    std::map<PackId, CardPackPtr> cardPacks;

    ranges::transform(spacks, std::inserter(cardPacks, cardPacks.begin()),
                      [&](const std::filesystem::path& spackFile)
                              -> std::pair<PackId, std::shared_ptr<CardPack>> {
                          auto packId = packIdGenerator->getNext();
                          auto cardPack = std::make_shared<CardPack>(spackFile, packId, cardIdGenerator, wordDB, tokenizer);
                          return {packId, cardPack};
                      });
    return cardPacks;
}

auto CardPackDB::setNameToCardPacks(std::map<PackId, CardPackPtr> idToCardPacks)
        -> std::map<std::string, CardPackPtr>
{
    std::map<std::string, CardPackPtr> nameToCardPacks;

    ranges::transform(idToCardPacks, std::inserter(nameToCardPacks, nameToCardPacks.begin()),
                      [&](const std::pair<PackId, CardPackPtr>& idCardPack)
                              -> std::pair<std::string, std::shared_ptr<CardPack>> {
                          return {idCardPack.second->getName(), idCardPack.second};
                      });
    return nameToCardPacks;
}

auto CardPackDB::traverseCards() const -> std::generator<CardAudio>
{
    for (const auto& [_, cardPack] : nameToCardPacks) {
        for (const auto& cardAudio : cardPack->getCards()) {
            co_yield cardAudio;
        }
    }
    co_return;
}

void CardPackDB::setupCards()
{
    ranges::transform(traverseCards(), std::inserter(cards, cards.begin()),
                      [](const CardAudio& cardAudio) -> std::pair<CardId, CardAudio> {
                          return {cardAudio.card->getCardId(), cardAudio};
                      });
}
} // namespace database
