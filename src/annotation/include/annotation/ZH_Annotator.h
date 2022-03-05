#pragma once

#include <dictionary/ZH_Dictionary.h>
#include <utils/StringU8.h>
#include <compare>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <span>
#include <vector>

class ZH_Annotator {
public:
    using CharacterSequence = std::vector<utl::CharU8>;
    using Combination = std::vector<int>;
    using ZH_dicItemVec = std::vector<ZH_Dictionary::Item>;

    ZH_Annotator(const utl::StringU8& _text,
                 const std::shared_ptr<ZH_Dictionary>& _dictionary,
                 const std::map<CharacterSequence, Combination>& _choices = {});
    auto Annotated() const -> const std::string&;

    struct Item {
        template <class string_t> Item(const string_t& _text) : text(_text), dicItemVec{} {}
        Item(const utl::StringU8& _text, const ZH_dicItemVec&& _dicItem)
            : text(_text), dicItemVec(_dicItem) {}

        auto operator<=>(const Item&) const -> std::weak_ordering;

        utl::StringU8 text;
        ZH_dicItemVec dicItemVec;
    };
    auto Items() const -> const std::vector<Item>&;
    auto UniqueItems() const -> std::set<Item>;
    auto Candidates() const -> const std::vector<std::vector<ZH_dicItemVec>>&;
    auto Chunks() const -> const std::vector<std::vector<std::vector<int>>>&;
    static auto get_combinations(const std::vector<std::vector<int>>& chunk)
        -> std::vector<std::vector<int>>;

    void SetAnnotationChoices(const std::map<CharacterSequence, Combination>& choices);
    void Reannotate();
    auto ContainsCharacterSequence(const CharacterSequence& charSeq) -> bool;

private:
    void annotate();
    const utl::StringU8 text;
    std::string annotated_text;
    const std::shared_ptr<ZH_Dictionary> dictionary;
    std::vector<Item> items;
    std::vector<std::vector<std::vector<int>>> chunks;
    std::vector<std::vector<ZH_dicItemVec>> candidates;

    std::map<CharacterSequence, Combination> choices;
};
