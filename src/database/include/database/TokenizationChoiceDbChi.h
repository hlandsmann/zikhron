#pragma once
#include "TokenizationChoiceDB.h"
#include <database/Card.h>
#include <database/CbdFwd.h>
#include <database/VideoDB.h>
#include <misc/Config.h>
#include <misc/Identifier.h>
#include <misc/TokenizationChoice.h>
#include <utils/StringU8.h>

#include <cstddef>
#include <filesystem>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace database {

class TokenizationChoiceDbChi : public TokenizationChoiceDB
{
    static constexpr auto s_tokenizationChoiceDBFile = "tokenizationChoices.zdb";
    static constexpr auto s_type = "tokenizationChoiceDB";

    using PackName = std::string;
    using IndicesInPack = std::set<std::size_t>;
    using PackPosition = std::pair<PackName, IndicesInPack>;

    struct TokenizationChoicePosition
    {
        TokenizationChoice tokenizationChoice;
        std::map<PackName, IndicesInPack> packPositions;
    };

public:
    TokenizationChoiceDbChi(std::shared_ptr<zikhron::Config> config,
                         const std::shared_ptr<database::CardPackDB>& cardPackDB,
                         const std::shared_ptr<database::VideoDB>& videoDB);
    void insertTokenization(const TokenizationChoice& choice, const std::shared_ptr<Card>& card);
    auto getChoicesForCard(CardId cardId) -> std::vector<TokenizationChoice>;
    [[nodiscard]] auto getChoicesForCards() const -> const std::map<CardId, TokenizationChoiceVec>&;
    void save();

private:
    void removeSimilarChoiceForCard(const TokenizationChoice& choice, const std::shared_ptr<Card>& card);
    void addChoiceForCard(const TokenizationChoice& choice, const std::shared_ptr<Card>& card);
    void syncIdsWithCardPackDB(const CardPackDB& cardPackDB, const database::VideoDB& videoDB);
    [[nodiscard]] auto serialize() const -> std::string;
    [[nodiscard]] static auto serializeChoice(const TokenizationChoice&) -> std::string;
    [[nodiscard]] static auto serializePackPosition(const PackPosition&) -> std::string;
    [[nodiscard]] static auto deserialize(const std::filesystem::path& dbFile) -> std::vector<TokenizationChoicePosition>;
    [[nodiscard]] static auto deserializeChoice(std::string_view sv) -> TokenizationChoice;
    [[nodiscard]] static auto deserializePackPosition(std::string_view sv) -> PackPosition;

    std::filesystem::path dbFile;

    std::vector<TokenizationChoicePosition> choices;
    std::map<CardId, TokenizationChoiceVec> choicesForCards;
};
} // namespace database
