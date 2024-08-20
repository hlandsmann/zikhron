#include "TokenizationChoiceDB.h"

#include <database/CardPackDB.h>
#include <database/CbdFwd.h>
#include <database/VideoDB.h>
#include <misc/Config.h>
#include <misc/Identifier.h>
#include <misc/TokenizationChoice.h>
#include <utils/StringU8.h>
#include <utils/format.h>
#include <utils/string_split.h>

#include <algorithm>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace ranges = std::ranges;

namespace database {
TokenizationChoiceDB::TokenizationChoiceDB(std::shared_ptr<zikhron::Config> config,
                                           const std::shared_ptr<database::CardPackDB>& cardPackDB,
                                           const std::shared_ptr<database::VideoDB>& videoDB)
    : dbFile{config->DatabaseDirectory() / s_tokenizationChoiceDBFile}
    , choices{deserialize(dbFile)}
{
    syncIdsWithCardPackDB(*cardPackDB, *videoDB);
}

void TokenizationChoiceDB::syncIdsWithCardPackDB(const CardPackDB& cardPackDB, const database::VideoDB& videoDB)
{
    for (const auto& tokenizationChoicePosition : choices) {
        const auto& choice = tokenizationChoicePosition.tokenizationChoice;
        for (const auto& [packName, indicesInPack] : tokenizationChoicePosition.packPositions) {
            std::string audioError;
            std::string videoError;
            try {
                const auto& pack = cardPackDB.getCardPack(packName);
                for (const auto indexInPack : indicesInPack) {
                    const auto& cardAudio = pack->getCardAudioByIndex(indexInPack);
                    choicesForCards[cardAudio.card->getCardId()].push_back(choice);
                }
                continue;
            } catch (const std::exception& e) {
                audioError = e.what();
            }
            try {
                const auto& video = videoDB.getVideo(packName);
                const auto& subs = video->getActiveSubtitle();
                for (const auto indexInPack : indicesInPack) {
                    const auto& joinedSub = subs->getJoinedSubAtIndex(indexInPack);
                    choicesForCards[joinedSub.card->getCardId()].push_back(choice);
                }
                continue;
            } catch (const std::exception& e) {
                videoError = e.what();
            }
            spdlog::error("Tokenization: pack {} not found", packName);
            spdlog::error("AudioPack error: {}", audioError);
            spdlog::error("Video error: {}", videoError);
        }
    }
}

void TokenizationChoiceDB::insertTokenization(const TokenizationChoice& choice, const std::shared_ptr<database::Card>& card)
{
    removeSimilarChoiceForCard(choice, card);
    addChoiceForCard(choice, card);
}

void TokenizationChoiceDB::removeSimilarChoiceForCard(const TokenizationChoice& choice, const std::shared_ptr<database::Card>& card)
{
    auto& choicesForCard = choicesForCards[card->getCardId()];
    auto oldChoiceForCardIt = ranges::find_if(choicesForCard,
                                              [&choice](const TokenizationChoice& tc) -> bool {
                                                  return utl::concanateStringsU8(tc) == utl::concanateStringsU8(choice);
                                              });
    if (oldChoiceForCardIt == choicesForCard.end()) {
        return;
    }
    auto oldChoice = *oldChoiceForCardIt;
    choicesForCard.erase(oldChoiceForCardIt);

    auto oldChoiceIt = ranges::find(choices, oldChoice, &TokenizationChoicePosition::tokenizationChoice);
    if (oldChoiceIt == choices.end()) {
        throw std::runtime_error(fmt::format("Bad TokenizationChoiceDB structure, {} not found", fmt::join(oldChoice, "/")));
    }
    auto& packPosition = oldChoiceIt->packPositions;
    packPosition.at(card->getPackName()).erase(card->getIndexInPack());
    if (packPosition.at(card->getPackName()).empty()) {
        packPosition.erase(card->getPackName());
    }
    if (packPosition.empty()) {
        choices.erase(oldChoiceIt);
    }
}

void TokenizationChoiceDB::addChoiceForCard(const TokenizationChoice& choice, const std::shared_ptr<database::Card>& card)
{
    auto choiceIt = ranges::find(choices, choice, &TokenizationChoicePosition::tokenizationChoice);
    if (choiceIt == choices.end()) {
        choices.push_back({.tokenizationChoice = choice,
                           .packPositions = {}});
        choiceIt = std::prev(choices.end());
    }
    auto& location = choiceIt->packPositions;
    location[card->getPackName()].insert(card->getIndexInPack());
    choicesForCards[card->getCardId()].push_back(choice);
}

auto TokenizationChoiceDB::getChoicesForCard(CardId cardId) -> std::vector<TokenizationChoice>
{
    if (choicesForCards.contains(cardId)) {
        return choicesForCards.at(cardId);
    }
    return {};
}

auto TokenizationChoiceDB::getChoicesForCards() const -> const std::map<CardId, TokenizationChoiceVec>&
{
    return choicesForCards;
}

void TokenizationChoiceDB::save()
{
    if (choices.empty()) {
        return;
    }
    auto out = std::ofstream{dbFile};
    out << serialize();
    spdlog::info("Saved tokenization choices");
}

auto TokenizationChoiceDB::serialize() const -> std::string
{
    std::string content;
    content += fmt::format("{};version:1.0\n", s_type);
    for (const auto& choice : choices) {
        content += fmt::format("{}\n", serializeChoice(choice.tokenizationChoice));
        for (const auto& position : choice.packPositions) {
            content += fmt::format("{}\n", serializePackPosition(position));
        }
        content += "\n";
    }

    return content;
}

auto TokenizationChoiceDB::serializeChoice(const TokenizationChoice& choice) -> std::string
{
    return fmt::format("{}/", fmt::join(choice, "/"));
}

auto TokenizationChoiceDB::serializePackPosition(const PackPosition& position) -> std::string
{
    const auto& [packName, indicesInPack] = position;
    return fmt::format("{}/{}/", packName, fmt::join(indicesInPack, "/"));
}

auto TokenizationChoiceDB::deserialize(const std::filesystem::path& dbFile) -> std::vector<TokenizationChoicePosition>
{
    std::vector<TokenizationChoicePosition> choices;
    if (!std::filesystem::exists(dbFile)) {
        return {};
    }

    auto content = utl::load_string_file(dbFile);
    auto rest = std::string_view{content};
    auto dbType = utl::split_front(rest, ';');
    if (dbType != s_type) {
        throw std::runtime_error(fmt::format("Invalid Tokenization Choice Database. Failed to parse type: {}", dbType));
    }
    auto versionString = utl::split_front(rest, ':');
    if (versionString != "version") {
        throw std::runtime_error(fmt::format("Expected \"version\", got: {}", versionString));
    }
    auto version = utl::split_front(rest, '\n');
    if (version != "1.0") {
        throw std::runtime_error(fmt::format("Only version 1.0 is supported, got: {}", version));
    }

    while (!rest.empty()) {
        auto choiceSV = utl::split_front(rest, '\n');
        auto choice = deserializeChoice(choiceSV);

        std::map<PackName, IndicesInPack> packPositions;
        auto packPositionSV = utl::split_front(rest, '\n');
        while (!packPositionSV.empty()) {
            auto packPosition = deserializePackPosition(packPositionSV);
            packPositions.insert(std::move(packPosition));
            packPositionSV = utl::split_front(rest, '\n');
        }
        choices.push_back({.tokenizationChoice = std::move(choice),
                           .packPositions = std::move(packPositions)});
    }
    return choices;
}

auto TokenizationChoiceDB::deserializeChoice(std::string_view sv) -> TokenizationChoice
{
    auto rest = sv;
    TokenizationChoice choice;
    while (!rest.empty()) {
        auto token = utl::split_front(rest, '/');
        choice.emplace_back(token);
    }
    return choice;
}

auto TokenizationChoiceDB::deserializePackPosition(std::string_view sv) -> PackPosition
{
    auto rest = sv;
    PackPosition packPosition;
    utl::StringU8 packName = utl::split_front(rest, '/');
    IndicesInPack indicesInPack;
    while (!rest.empty()) {
        auto index = std::string{utl::split_front(rest, '/')};
        indicesInPack.insert(std::stoul(index));
    }
    return {packName, indicesInPack};
}

} // namespace database
