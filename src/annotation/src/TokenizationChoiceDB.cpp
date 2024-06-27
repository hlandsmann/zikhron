#include "TokenizationChoiceDB.h"

#include "AnnotationFwd.h"

#include <misc/Config.h>
#include <misc/Identifier.h>
#include <utils/StringU8.h>
#include <utils/format.h>
#include <utils/string_split.h>

#include <algorithm>
#include <filesystem>
#include <iterator>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace ranges = std::ranges;

namespace annotation {
TokenizationChoiceDB::TokenizationChoiceDB(std::shared_ptr<zikhron::Config> config)
    : dbFile{config->DatabaseDirectory() / s_tokenizationChoiceDBFile}
    , choices{deserialize(dbFile)}
{}

void TokenizationChoiceDB::insertTokenization(const TokenizationChoice& choice, CardPtr card)
{
    // clang-format off
    auto oldChoiceIt = ranges::find_if(choices,
                                       [&choice](const TokenizationChoice& tc) -> bool {
                                           return utl::concanateStringsU8(tc) == utl::concanateStringsU8(choice);
                                       }, &TokenizationChoicePosition::tokenizationChoice);
    // clang-format on
    if (oldChoiceIt != choices.end()) {
        oldChoiceIt->tokenizationChoice = choice;
    } else {
        choices.push_back({.tokenizationChoice = choice,
                           .packPositions = {}});
        oldChoiceIt = std::prev(choices.end());
    }

    auto& location = oldChoiceIt->packPositions;
    location[card->getPackName()].insert(card->getIndexInPack());

    auto& choicesForCard = choicesForCards[card->getCardId()];
    auto oldChoiceForCardIt = ranges::find_if(choicesForCard,
                                              [&choice](const TokenizationChoice& tc) -> bool {
                                                  return utl::concanateStringsU8(tc) == utl::concanateStringsU8(choice);
                                              });
    if (oldChoiceForCardIt != choicesForCard.end()) {
        choicesForCard.erase(oldChoiceForCardIt);
    }
    choicesForCard.push_back(choice);
}

auto TokenizationChoiceDB::getChoicesForCard(CardId cardId) -> std::vector<TokenizationChoice>
{
    if (choicesForCards.contains(cardId)) {
        return choicesForCards.at(cardId);
    }
    return {};
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
        auto choice = parseChoice(choiceSV);

        std::map<PackName, IndicesInPack> packPositions;
        auto packPositionSV = utl::split_front(rest, '\n');
        while (!packPositionSV.empty()) {
            auto packPosition = parsePackPosition(packPositionSV);
            packPositions.insert(std::move(packPosition));
            packPositionSV = utl::split_front(rest, '\n');
        }
        choices.push_back({.tokenizationChoice = std::move(choice),
                           .packPositions = std::move(packPositions)});
    }
    return choices;
}

auto TokenizationChoiceDB::parseChoice(std::string_view sv) -> TokenizationChoice
{
    auto rest = sv;
    TokenizationChoice choice;
    while (!rest.empty()) {
        auto token = utl::split_front(rest, '/');
        choice.emplace_back(token);
    }
    return choice;
}

auto TokenizationChoiceDB::parsePackPosition(std::string_view sv) -> PackPosition
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

} // namespace annotation
