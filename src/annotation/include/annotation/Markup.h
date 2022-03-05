#pragma once

#include <annotation/Ease.h>
#include <annotation/TextCard.h>
#include <utils/StringU8.h>
#include <functional>
#include <iosfwd>
#include <stack>

#include <annotation/ZH_Annotator.h>
#include <dictionary/ZH_Dictionary.h>
namespace markup {

class Word {
    std::string rawWord;
    auto joinCharactersNonBreakable(const utl::StringU8& word) const -> std::string;
    auto utf8ByteLengthOfWord(const utl::StringU8& word) const -> size_t;
    auto lengthOfWord(const utl::StringU8& word) const -> size_t;

public:
    Word(const utl::StringU8& word, uint32_t color, uint32_t backGroundColor);
    Word(const utl::StringU8& word);
    Word(const Word&) = default;
    Word(Word&&) = default;

    Word& operator=(const Word&& word);
    operator std::string() const;

    void setColor(uint32_t color);
    void setBackgroundColor(uint32_t backgroundColor);
    auto vLength() const -> size_t { return virtualLength; }
    auto byteLength() const -> size_t { return utf8ByteLength; }
    auto isMarkup() const -> bool { return markup; }

private:
    auto applyStyle(const std::string& str) const -> std::string;
    size_t virtualLength = 0;
    size_t utf8ByteLength = 0;
    bool markup = false;
    uint32_t color = 0;
    uint32_t backGroundColor = 0;
    std::string styledWord;
};

class Paragraph {
    struct AnnotationChunk;

public:
    using value_type = Word;
    using vocable_pronounciation_meaning_t = std::tuple<std::string, std::string, std::string>;
    static auto textFromCard(const Card&) -> utl::StringU8;

    Paragraph(std::unique_ptr<Card> card);

    auto get() const -> std::string;
    auto getFragments() const -> std::vector<std::string>;
    auto getWordStartPosition(int pos, const std::vector<int>& positions) const -> int;
    auto getWordIndex(int pos, const std::vector<int>& positions) const -> std::size_t;
    auto BytePositions() const -> const std::vector<int>& { return bytePositions; };
    auto Utf8Positions() const -> const std::vector<int>& { return utf8Positions; };
    auto fragmentStartPos(size_t fragment, const std::vector<int>& positions) const -> int;

    void updateAnnotationColoring();
    void highlightWordAtPosition(int pos, const std::vector<int>& positions);
    void highlightAnnotationAtPosition(int pos);

    struct AnnotationPossibilities {
        std::string activeChoice;
        std::vector<std::string> unmarked;
        std::vector<std::string> marked;
        size_t pos{};
        std::vector<std::vector<int>> combinations;
        std::vector<utl::CharU8> characters;
    };
    auto getAnnotationPossibilities(int pos) -> AnnotationPossibilities;
    void undoChange();
    auto wordFromPosition(int pos, const std::vector<int>& positions) const
        -> const ZH_Annotator::ZH_dicItemVec;
    void setupVocables(std::vector<std::pair<ZH_Dictionary::Item, uint>>&&);
    // auto getVocableString() const -> std::string;
    auto getVocables() const -> const std::vector<vocable_pronounciation_meaning_t>&;
    // auto getVocablePositions() const -> const std::vector<int>&;
    auto getRelativeOrderedEaseList(const std::map<uint, Ease>&) const
        -> std::vector<std::pair<uint, Ease>>;
    auto getRestoredOrderOfEaseList(const std::vector<Ease>&) const -> std::map<uint, Ease>;

private:
    auto getAnnotationChunkFromPosition(size_t pos)
        -> std::optional<std::reference_wrapper<AnnotationChunk>>;
    auto calculate_positions(const std::vector<Word>& words, size_t (Word::*len)() const) const
        -> std::vector<int>;

    std::unique_ptr<Card> card;

    struct WordState {
        std::size_t index;
        Word word;
    };

    struct AnnotationChunk {
        std::vector<std::reference_wrapper<Word>> words;
        std::vector<utl::CharU8> characters;
        std::vector<std::vector<int>> chunk;
        size_t posBegin = 0;
        size_t posEnd = 0;
        void clear() {
            words.clear();
            characters.clear();
            chunk.clear();
            posBegin = 0;
            posEnd = 0;
        }
    };
    std::vector<AnnotationChunk> annotationChunks;

    std::stack<WordState> preChanges;
    std::vector<Word> words;
    std::vector<vocable_pronounciation_meaning_t> vocables;
    std::vector<int> utf8Positions;
    std::vector<int> bytePositions;
    std::vector<std::span<const Word>> fragments;

    std::vector<std::pair<ZH_Dictionary::Item, uint>> vocables_id;
    // std::string vocableString;
    // std::vector<int> vocablePositions;

    constexpr static std::array markingColors_red = {0x772222, 0xAA7777};
    constexpr static std::array markingColors_green = {0x227722, 0x77AA77};
    constexpr static std::array markingColors_blue = {0x222277, 0x7777AA};
};
}  // namespace markup

inline std::ostream& operator<<(std::ostream& os, const markup::Word& word) {
    os << std::string(word);
    return os;
}

inline std::string operator+(std::string&& str, const markup::Word& word) {
    return str + std::string(word);
}
