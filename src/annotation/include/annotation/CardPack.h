#pragma once
#include "Card.h"
#include "Tokenizer.h"
#include "WordDB.h"

#include <misc/Config.h>
#include <misc/Identifier.h>

#include <filesystem>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace annotation {

struct CardAudio
{
    std::filesystem::path audioFile;
    std::shared_ptr<Card> card;
    double start;
    double end;
};

enum class CardPackType {
    storypack,
    snippetpack,
};

class CardPack
{
public:
    CardPack(std::filesystem::path filename,
             PackId packid,
             std::shared_ptr<WordDB> wordDB,
             std::shared_ptr<Tokenizer> tokenizer);
    [[nodiscard]] auto getCards() const -> const std::vector<CardAudio>&;

    [[nodiscard]] auto getFirstCard() const -> const CardAudio&;
    [[nodiscard]] auto getLastCard() const -> const CardAudio&;
    [[nodiscard]] auto getNextCard(const CardPtr& card) const -> std::optional<CardAudio>;
    [[nodiscard]] auto getPreviousCard(const CardPtr& card) const -> std::optional<CardAudio>;

private:
    void deserialize();
    [[nodiscard]] auto serialize() const -> std::string;
    std::filesystem::path filename;
    PackId packId;
    std::shared_ptr<WordDB> wordDB;
    std::shared_ptr<Tokenizer> tokenizer;

    std::vector<CardAudio> cards;

    std::string name;
    CardPackType type{};
};

using CardPackPtr = std::shared_ptr<CardPack>;

} // namespace annotation
