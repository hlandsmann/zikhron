#pragma once
#include "AnnotationFwd.h"
#include "Card.h"

#include <misc/Config.h>
#include <misc/Identifier.h>
#include <utils/StringU8.h>

#include <cstddef>
#include <filesystem>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace annotation {

class TokenizationChoiceDB
{
    static constexpr auto s_tokenizationChoiceDBFile = "tokenizationChoices.zdb";
    static constexpr auto s_type = "tokenizationChoiceDB";

    using PackName = std::string;
    using IndicesInPack = std::set<std::size_t>;
    using PackPosition = std::map<PackName, IndicesInPack>;

public:
    TokenizationChoiceDB(std::shared_ptr<zikhron::Config> config);
    void insertTokenization(const TokenizationChoice& choice, std::shared_ptr<Card> card);
    auto getChoicesForCard(CardId cardId) -> std::vector<TokenizationChoice>;

private:
    [[nodiscard]] static auto deserialize(const std::filesystem::path& dbFile) -> std::map<TokenizationChoice, PackPosition>;
    [[nodiscard]] static auto parseChoice(std::string_view sv) -> TokenizationChoice;
    std::filesystem::path dbFile;

    std::map<TokenizationChoice, PackPosition> choices;
    std::map<CardId, TokenizationChoiceVec> choicesForCards;
};
} // namespace annotation
