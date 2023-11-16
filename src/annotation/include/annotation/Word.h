#pragma once
#include <annotation/CardDB.h>
#include <annotation/Ease.h>
#include <annotation/ZH_Tokenizer.h>
#include <dictionary/ZH_Dictionary.h>
#include <misc/Identifier.h>
#include <utils/StringU8.h>

#include <cstddef>
#include <cstdint>
#include <string>

#include <sys/types.h>
namespace markup {

class Word
{
    std::string rawWord;
    [[nodiscard]] static auto joinCharactersNonBreakable(const utl::StringU8& word) -> std::string;
    [[nodiscard]] static auto utf8ByteLengthOfWord(const utl::StringU8& word) -> size_t;
    [[nodiscard]] static auto lengthOfWord(const utl::StringU8& word) -> size_t;

public:
    Word(const utl::StringU8& word, uint32_t color, uint32_t backGroundColor);
    Word(const utl::StringU8& word);
    Word(const Word&) = default;
    Word(Word&&) = default;
    ~Word() = default;

    auto operator=(const Word& word) noexcept -> Word& = default;
    auto operator=(Word&& word) noexcept -> Word& = default;
    operator std::string() const;

    void setColor(uint32_t color);
    void setBackgroundColor(uint32_t backgroundColor);
    [[nodiscard]] auto vLength() const -> size_t { return virtualLength; }
    [[nodiscard]] auto byteLength() const -> size_t { return utf8ByteLength; }
    [[nodiscard]] auto isMarkup() const -> bool { return markup; }

private:
    [[nodiscard]] auto applyStyle(const std::string& str) const -> std::string;
    size_t virtualLength = 0;
    size_t utf8ByteLength = 0;
    bool markup = false;
    uint32_t color = 0;
    uint32_t backGroundColor = 0;
    std::string styledWord;
};
} // namespace markup
