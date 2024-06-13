#pragma once
#include "CardPack.h"
#include "Tokenizer.h"
#include "WordDB.h"

#include <misc/Config.h>
#include <misc/Identifier.h>

#include <filesystem>
#include <generator>
#include <map>
#include <memory>
#include <vector>

namespace annotation {
class CardPackDB
{
    static constexpr auto s_packSubdirectory = "pack";
    static constexpr auto s_spackExtension = ".spkg";

public:
    CardPackDB(std::shared_ptr<zikhron::Config> config,
               std::shared_ptr<WordDB> wordDB);
    [[nodiscard]] auto getCards() const -> const std::map<CardId, CardAudio>&;
    [[nodiscard]] auto getTokenizer() const -> std::shared_ptr<annotation::Tokenizer>;
    [[nodiscard]] auto getAnnotationAlternativesForCard(CardId) const -> std::vector<Alternative>;
    [[nodiscard]] auto getCardAtCardId(CardId) const -> const CardAudio&;
    [[nodiscard]] auto getCardPackForCardId(CardId) const -> CardPackPtr;

private:
    auto loadCardPacks(std::filesystem::path directory) -> std::vector<CardPackPtr>;
    [[nodiscard]] auto traverseCards() const -> std::generator<CardAudio>;
    void setupCards();
    std::shared_ptr<WordDB> wordDB;
    std::shared_ptr<Tokenizer> tokenizer;
    std::vector<CardPackPtr> cardPacks;
    std::map<CardId, CardAudio> cards;
};
} // namespace annotation
