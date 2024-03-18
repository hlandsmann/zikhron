#pragma once
#include "ZH_Tokenizer.h"

#include <dictionary/ZH_Dictionary.h>
#include <misc/Identifier.h>
#include <utils/StringU8.h>

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace annotation {
enum class NoBreak {
    front,
    back,
    none
};

class Token
{
public:
    using DictionaryEntry = std::optional<std::shared_ptr<ZH_Dictionary::Entry>>;
    using ZH_dicItemVec = ZH_Tokenizer::ZH_dicItemVec;

    Token() = default;
    Token(utl::StringU8 value, ZH_dicItemVec dictionaryEntries);
    [[nodiscard]] auto getValue() const -> utl::StringU8;
    [[nodiscard]] auto getNoBreak() const -> NoBreak;
    [[nodiscard]] auto getColorId() const -> ColorId;
    void setColorId(ColorId colorId);
    [[nodiscard]] auto getVocableId() const -> std::optional<VocableId>;
    void setVocableId(VocableId vocableId);
    [[nodiscard]] auto getDictionaryEntries() const -> const ZH_dicItemVec&;

    [[nodiscard]] auto string() const -> std::string;
    operator std::string() const;

private:
    utl::StringU8 value;
    ColorId colorId{0};
    NoBreak noBreak{NoBreak::none};
    std::size_t dictionaryEntryIndex{0};
    ZH_dicItemVec dictionaryEntries;
    std::optional<VocableId> vocableId;
};

auto tokenVectorFromString(const std::string& str, ColorId colorId) -> std::vector<Token>;
} // namespace annotation
