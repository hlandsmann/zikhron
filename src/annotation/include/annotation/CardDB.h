#pragma once
#include "Card.h"
#include "Tokenizer.h"
#include "WordDB.h"

#include <dictionary/ZH_Dictionary.h>
#include <misc/Config.h>
#include <misc/Identifier.h>
#include <unicode/unistr.h>
#include <utils/StringU8.h>

#include <filesystem>
#include <map>
#include <memory>
#include <string_view>

#include <sys/types.h>

namespace annotation {

class CardDB
{
public:
    static constexpr std::string_view s_cardSubdirectory = "cards";

    using CardPtr = std::shared_ptr<Card>;
    using CardPtrConst = std::shared_ptr<const Card>;
    using CharacterSequence = Card::CharacterSequence;

    CardDB() = default;
    CardDB(std::shared_ptr<zikhron::Config> config, std::shared_ptr<WordDB> wordDB);
    static auto loadFromDirectory(const std::filesystem::path& directoryPath,
                                  const std::shared_ptr<WordDB>& wordDB,
                                  const std::shared_ptr<annotation::Tokenizer>& tokenizer)
            -> std::map<CardId, CardPtr>;

    [[nodiscard]] auto get() const -> const std::map<CardId, CardPtr>&;
    [[nodiscard]] auto atId(CardId) -> CardPtr&;
    [[nodiscard]] auto atId(CardId) const -> CardPtrConst;

    [[nodiscard]] auto getTokenizer() const -> std::shared_ptr<annotation::Tokenizer> { return tokenizer; }

    void getAnnotationTokens(CardId cardId);

private:
    std::shared_ptr<zikhron::Config> config;
    std::shared_ptr<WordDB> wordDB;
    std::shared_ptr<annotation::Tokenizer> tokenizer;
    std::map<CardId, CardPtr> cards;
};

} // namespace annotation
