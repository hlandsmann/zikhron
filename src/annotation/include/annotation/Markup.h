#pragma once
#include <annotation/CardDB.h>
#include <annotation/Ease.h>
#include <annotation/Word.h>
#include <annotation/ZH_Tokenizer.h>
#include <dictionary/ZH_Dictionary.h>
#include <misc/Identifier.h>
#include <utils/StringU8.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iosfwd>
#include <map>
#include <memory>
#include <optional>
#include <span>
#include <stack>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <sys/types.h>
namespace markup {

class Paragraph
{
    struct AnnotationChunk;

public:
    using value_type = Word;
    using vocable_pronounciation_meaning_t = std::tuple<std::string, std::string, std::string>;
    // ToDo: move that function to TextCard

    Paragraph(std::shared_ptr<Card> card);
    Paragraph(std::shared_ptr<Card> card, std::vector<VocableId>&& vocableIds);
    [[nodiscard]] auto get() const -> std::string;
    [[nodiscard]] auto getFragments() const -> std::vector<std::string>;
    [[nodiscard]] auto getWordStartPosition(int pos, const std::vector<int>& positions) const -> int;
    [[nodiscard]] auto getWordIndex(int pos, const std::vector<int>& positions) const -> std::size_t;
    [[nodiscard]] auto BytePositions() const -> const std::vector<int>& { return bytePositions; };
    [[nodiscard]] auto Utf8Positions() const -> const std::vector<int>& { return utf8Positions; };
    [[nodiscard]] auto fragmentStartPos(size_t fragment, const std::vector<int>& positions) const -> int;

    void updateAnnotationColoring();
    void highlightWordAtPosition(int pos, const std::vector<int>& positions);
    void highlightAnnotationAtPosition(int pos);

    struct AnnotationPossibilities
    {
        std::string activeChoice;
        std::vector<std::string> unmarked;
        std::vector<std::string> marked;
        size_t pos{};
        std::vector<std::vector<int>> combinations;
        std::vector<utl::CharU8> characters;
    };
    auto getAnnotationPossibilities(int pos) -> AnnotationPossibilities;
    void undoChange();
    [[nodiscard]] auto wordFromPosition(int pos, const std::vector<int>& positions) const
            -> const ZH_Tokenizer::ZH_dicItemVec;
    [[nodiscard]] auto getVocableChoiceFromPosition(int pos, const std::vector<int>& positions) const
            -> ZH_Dictionary::Entry;
    void setupVocables(const std::map<VocableId, Ease>&);
    [[nodiscard]] auto getVocables() const -> const std::vector<vocable_pronounciation_meaning_t>&;
    [[nodiscard]] auto getRelativeOrderedEaseList(const std::map<VocableId, Ease>&) const
            -> std::vector<std::pair<VocableId, Ease>>;
    [[nodiscard]] auto getRestoredOrderOfEaseList(const std::vector<Ease>&) const -> std::map<VocableId, Ease>;

private:
    auto getAnnotationChunkFromPosition(size_t pos)
            -> std::optional<std::reference_wrapper<AnnotationChunk>>;
    [[nodiscard]] auto calculate_positions(size_t (Word::*len)() const) const -> std::vector<int>;

    std::shared_ptr<Card> card;

    struct WordState
    {
        std::size_t index;
        Word word;
    };

    struct AnnotationChunk
    {
        std::vector<std::reference_wrapper<Word>> words;
        std::vector<utl::CharU8> characters;
        std::vector<std::vector<int>> chunk;
        size_t posBegin = 0;
        size_t posEnd = 0;
        void clear()
        {
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

    std::vector<VocableId> vocableIds;
    std::vector<VocableId> activeVocables;

    // std::vector<std::pair<ZH_Dictionary::Entry, uint>> vocables_id;

    constexpr static std::array markingColors_red = {0x772222, 0xAA7777};
    constexpr static std::array markingColors_green = {0x227722, 0x77AA77};
    constexpr static std::array markingColors_blue = {0x222277, 0x7777AA};
};
} // namespace markup

inline auto operator<<(std::ostream& os, const markup::Word& word) -> std::ostream&
{
    os << std::string(word);
    return os;
}

inline auto operator+(std::string&& str, const markup::Word& word) -> std::string
{
    return str + std::string(word);
}
