#pragma once
#include "CardPack.h"

#include <annotation/Tokenizer.h>
#include <database/WordDB.h>
#include <misc/Config.h>
#include <misc/Identifier.h>

#include <filesystem>
#include <generator>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace database {
class CardPackDB
{
    static constexpr auto s_packSubdirectory = "pack";
    static constexpr auto s_spackExtension = ".spkg";

public:
    CardPackDB(std::shared_ptr<zikhron::Config> config,
               std::shared_ptr<WordDB> wordDB,
               std::shared_ptr<annotation::Tokenizer> tokenizer);
    [[nodiscard]] auto getCards() const -> const std::map<CardId, CardAudio>&;
    [[nodiscard]] auto getTokenizer() const -> std::shared_ptr<annotation::Tokenizer>;
    [[nodiscard]] auto getAnnotationAlternativesForCard(CardId) const -> std::vector<annotation::Alternative>;
    [[nodiscard]] auto getCardAtCardId(CardId) const -> const CardAudio&;
    [[nodiscard]] auto getCardPackForCardId(CardId) const -> CardPackPtr;
    [[nodiscard]] auto getCardPack(const std::string& packName) const -> CardPackPtr;

private:
    [[nodiscard]] static auto loadCardPacks(const std::filesystem::path& directory,
                                            const std::shared_ptr<WordDB>& wordDB,
                                            const std::shared_ptr<annotation::Tokenizer>& tokenizer)
            -> std::map<std::string, CardPackPtr>;
    [[nodiscard]] auto traverseCards() const -> std::generator<CardAudio>;
    void setupCards();
    std::shared_ptr<WordDB> wordDB;
    std::shared_ptr<annotation::Tokenizer> tokenizer;
    std::map<std::string /* name */, CardPackPtr> cardPacks;

    std::map<CardId, CardAudio> cards;
};
} // namespace database
