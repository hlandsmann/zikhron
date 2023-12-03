#pragma once
#include "ZH_Tokenizer.h"

#include <dictionary/ZH_Dictionary.h>
#include <utils/StringU8.h>

#include <cstddef>
#include <memory>
#include <optional>

namespace annotation {
enum ColorId : unsigned;
enum class NoBreak {
    front,
    back,
    none
};

class Token
{
public:
    using DictionaryEntry = std::optional<std::shared_ptr<ZH_Dictionary::Entry>>;
    using ZH_dicItemVec = std::optional<ZH_Tokenizer::ZH_dicItemVec>;

    Token() = default;
    Token(utl::StringU8 value, ZH_dicItemVec dictionaryEntries);
    [[nodiscard]] auto getValue() const -> utl::StringU8;
    [[nodiscard]] auto getNoBreak() const -> NoBreak;
    [[nodiscard]] auto getColorId(ColorId maxId) const -> ColorId;

private:
    utl::StringU8 value;
    ColorId color{0};
    NoBreak noBreak{NoBreak::none};
    std::size_t dictionaryEntryIndex{0};
    ZH_dicItemVec dictionaryEntries;
};

} // namespace annotation
