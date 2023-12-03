#pragma once
#include "Card.h"
#include <dictionary/ZH_Dictionary.h>
#include <misc/Identifier.h>
#include <unicode/unistr.h>
#include <utils/StringU8.h>

#include <filesystem>
#include <map>
#include <memory>
#include <string_view>

#include <sys/types.h>

class CardDB
{
public:
    static constexpr std::string_view s_cardSubdirectory = "cards";

    using CardPtr = std::shared_ptr<Card>;
    using CardPtrConst = std::shared_ptr<const Card>;
    using CharacterSequence = Card::CharacterSequence;
    using Combination = Card::Combination;
    using AnnotationChoiceMap = Card::AnnotationChoiceMap;

    CardDB() = default;
    CardDB(const std::filesystem::path& directoryPath,
           std::shared_ptr<const ZH_Dictionary> dictionary,
           std::shared_ptr<const AnnotationChoiceMap> annotationChoices);
    static auto loadFromDirectory(const std::filesystem::path& directoryPath,
                                  const std::shared_ptr<const ZH_Dictionary>& dictionary,
                                  const std::shared_ptr<const AnnotationChoiceMap>& annotationChoices)
            -> std::map<CardId, CardPtr>;

    [[nodiscard]] auto get() const -> const std::map<CardId, CardPtr>&;
    [[nodiscard]] auto atId(CardId) -> CardPtr&;
    [[nodiscard]] auto atId(CardId) const -> CardPtrConst;

private:
    std::shared_ptr<const ZH_Dictionary> dictionary;
    std::shared_ptr<const AnnotationChoiceMap> annotationChoices;
    std::map<CardId, CardPtr> cards;
};
