#pragma once

#include <dictionary/ZH_Dictionary.h>
#include <utils/StringU8.h>

#include <compare>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

class ZH_Tokenizer
{
    utl::StringU8 text;

public:
    using CharacterSequence = std::vector<utl::CharU8>;
    using Combination = std::vector<int>;
    using AnnotationChoiceMap = std::map<CharacterSequence, Combination>;
    using ZH_dicItemVec = std::vector<ZH_Dictionary::Entry>;

    ZH_Tokenizer() = default;
    ZH_Tokenizer(utl::StringU8 text,
                 std::shared_ptr<const ZH_Dictionary> dictionary,
                 std::shared_ptr<const AnnotationChoiceMap> choices = {});
    [[nodiscard]] auto Annotated() const -> const std::string&;

    struct Token
    {
        template<class string_t>
        explicit Token(const string_t& _text)
            : text(_text)
        {}
        Token(utl::StringU8 _text, const ZH_dicItemVec&& _dicItem)
            : text(std::move(_text)), dicItemVec(_dicItem) {}

        auto operator<=>(const Token&) const -> std::weak_ordering;

        utl::StringU8 text;
        ZH_dicItemVec dicItemVec;
    };
    [[nodiscard]] auto Tokens() const -> const std::vector<Token>&;
    [[nodiscard]] auto UniqueItems() const -> std::set<Token>;
    [[nodiscard]] auto Candidates() const -> const std::vector<std::vector<ZH_dicItemVec>>&;
    [[nodiscard]] auto Chunks() const -> const std::vector<std::vector<std::vector<int>>>&;
    [[nodiscard]] auto Dictionary() const -> const std::shared_ptr<const ZH_Dictionary>&;
    static auto get_combinations(const std::vector<std::vector<int>>& chunk)
            -> std::vector<std::vector<int>>;

    // void SetAnnotationChoices(const std::map<CharacterSequence, Combination>& choices);
    void Reannotate();
    auto ContainsCharacterSequence(const CharacterSequence& charSeq) -> bool;

private:
    void annotate();
    std::shared_ptr<const ZH_Dictionary> dictionary;
    std::string annotated_text;
    std::vector<Token> tokens;
    std::vector<std::vector<std::vector<int>>> chunks;
    std::vector<std::vector<ZH_dicItemVec>> candidates;
    std::shared_ptr<const AnnotationChoiceMap> choices;
};
