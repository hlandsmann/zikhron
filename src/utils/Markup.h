#pragma once

#include <TextCard.h>
#include <utils/StringU8.h>
#include <functional>
#include <iosfwd>
#include <stack>

#include <Ease.h>
#include <ZH_Annotator.h>
#include <ZH_Dictionary.h>
namespace markup {

class Word {
    std::string rawWord;
    auto joinCharactersNonBreakable(const utl::StringU8& word) const -> std::string;
    auto lengthOfWord(const utl::StringU8& word) const -> int;

public:
    Word(const utl::StringU8& word, uint32_t color, uint32_t backGroundColor);
    Word(const utl::StringU8& word);
    Word(const Word&) = default;
    Word(Word&&) = default;

    Word& operator=(const Word&& word);
    operator std::string() const;

    void setColor(uint32_t color);
    void setBackgroundColor(uint32_t backgroundColor);
    auto vLength() const -> int { return virtualLength; }
    auto isMarkup() const -> bool { return markup; }

private:
    auto applyStyle(const std::string& str) const -> std::string;
    int virtualLength = 0;
    bool markup = false;
    uint32_t color = 0;
    uint32_t backGroundColor = 0;
    std::string styledWord;
};

class Paragraph {
    struct AnnotationChunk;

public:
    using value_type = Word;
    static utl::StringU8 textFromCard(const Card&);

    Paragraph() = default;
    Paragraph(const Card&, const std::shared_ptr<ZH_Dictionary>&);

    auto get() const -> std::string;
    auto getWordStartPosition(int pos) const -> int;
    auto getWordIndex(int pos) const -> std::size_t;

    void updateAnnotationColoring();
    void highlightWordAtPosition(int pos);
    void highlightAnnotationAtPosition(int pos);

    struct AnnotationPossibilities {
        std::vector<std::string> unmarked;
        std::vector<std::string> marked;
        int pos{};
        std::vector<std::vector<int>> combinations;
        std::vector<utl::ItemU8> characters;
    };
    auto getAnnotationPossibilities(int pos)
        -> AnnotationPossibilities;
    void undoChange();
    auto wordFromPosition(int pos) const -> const ZH_Annotator::ZH_dicItemVec;
    void setupVocables(std::vector<std::pair<ZH_Dictionary::Item, uint>>&&);
    auto getVocableString() const -> std::string;
    auto getVocablePositions() const -> const std::vector<int>&;
    auto getRelativeOrderedEaseList(const std::map<uint, Ease>&) const
        -> std::vector<std::pair<uint, Ease>>;
    auto getRestoredOrderOfEaseList(const std::vector<Ease>&) const -> std::map<uint, Ease>;

private:
    auto getAnnotationChunkFromPosition(int pos)
        -> std::optional<std::reference_wrapper<AnnotationChunk>>;

    struct WordState {
        std::size_t index;
        Word word;
    };

    struct AnnotationChunk {
        std::vector<std::reference_wrapper<Word>> words;
        std::vector<utl::ItemU8> characters;
        std::vector<std::vector<int>> chunk;
        int posBegin = 0;
        int posEnd = 0;
        void clear() {
            words.clear();
            characters.clear();
            chunk.clear();
            posBegin = 0;
            posEnd = 0;
        }
    };
    std::vector<AnnotationChunk> annotationChunks;
    std::unique_ptr<ZH_Annotator> zh_annotator;
    std::shared_ptr<ZH_Dictionary> zh_dictionary;

    std::stack<WordState> preChanges;
    std::vector<Word> words;
    std::vector<int> positions;

    std::vector<std::pair<ZH_Dictionary::Item, uint>> vocables_id;
    std::string vocableString;
    std::vector<int> vocablePositions;

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
// inline std::string& operator+=(std::string& str, const markup::Word& word) {
//     str += std::string(word);
//     return str;
// }
