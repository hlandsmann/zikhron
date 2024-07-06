#pragma once
#include "CbdFwd.h"

#include <annotation/Tokenizer.h>
#include "WordDB.h"
#include <misc/Config.h>
#include <misc/Identifier.h>

#include <cstddef>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace database {

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
             std::shared_ptr<annotation::Tokenizer> tokenizer);
    [[nodiscard]] auto getCards() const -> const std::vector<CardAudio>&;

    [[nodiscard]] auto getCardByIndex(std::size_t index) const -> const CardAudio&;
    [[nodiscard]] auto getFirstCard() const -> const CardAudio&;
    [[nodiscard]] auto getLastCard() const -> const CardAudio&;
    [[nodiscard]] auto getNextCard(const CardPtr& card) const -> std::optional<CardAudio>;
    [[nodiscard]] auto getPreviousCard(const CardPtr& card) const -> std::optional<CardAudio>;
    [[nodiscard]] auto getName() const -> std::string;

private:
    void deserialize();
    [[nodiscard]] auto serialize() const -> std::string;
    std::filesystem::path filename;
    PackId packId;
    std::shared_ptr<WordDB> wordDB;
    std::shared_ptr<annotation::Tokenizer> tokenizer;

    std::vector<CardAudio> cards;

    std::string name;
    CardPackType type{};
};

using CardPackPtr = std::shared_ptr<CardPack>;

} // namespace cbd
