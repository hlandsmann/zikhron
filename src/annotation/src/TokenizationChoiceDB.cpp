#include "TokenizationChoiceDB.h"

#include "AnnotationFwd.h"

#include <misc/Config.h>
#include <misc/Identifier.h>
#include <utils/StringU8.h>
#include <utils/format.h>
#include <utils/string_split.h>

#include <algorithm>
#include <filesystem>
#include <map>
#include <memory>
#include <stdexcept>
#include <string_view>
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
                                       }, &decltype(choices)::value_type::first);
    // clang-format on
    if (oldChoiceIt != choices.end() && oldChoiceIt->first != choice) {
        choices[choice] = oldChoiceIt->second;
        choices.erase(oldChoiceIt);
    }

    auto& location = choices[choice];
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

auto TokenizationChoiceDB::deserialize(const std::filesystem::path& dbFile) -> std::map<TokenizationChoice, PackPosition>
{
    std::map<TokenizationChoice, PackPosition> choices;
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
    }
    return choices;
}

auto parseChoice(std::string_view sv) -> TokenizationChoice
{
    return {};
}
} // namespace annotation
