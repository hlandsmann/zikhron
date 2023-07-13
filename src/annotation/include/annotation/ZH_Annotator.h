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
    utl::StringU8 text;

public:
    using CharacterSequence = std::vector<utl::CharU8>;
    using Combination = std::vector<int>;
    using ZH_dicItemVec = std::vector<ZH_Dictionary::Entry>;

    ZH_Annotator() = default;
    ZH_Annotator(const utl::StringU8& _text,
                 const std::shared_ptr<const ZH_Dictionary>& _dictionary,
                 const std::map<CharacterSequence, Combination>& _choices = {});
    [[nodiscard]] auto Annotated() const -> const std::string&;

    struct Item {
        template <class string_t> explicit Item(const string_t& _text) : text(_text) {}
        Item(utl::StringU8 _text, const ZH_dicItemVec&& _dicItem)
            : text(std::move(_text)), dicItemVec(_dicItem) {}

        auto operator<=>(const Item&) const -> std::weak_ordering;

        utl::StringU8 text;
        ZH_dicItemVec dicItemVec;
    };
    [[nodiscard]] auto Items() const -> const std::vector<Item>&;
    [[nodiscard]] auto UniqueItems() const -> std::set<Item>;
    [[nodiscard]] auto Candidates() const -> const std::vector<std::vector<ZH_dicItemVec>>&;
    [[nodiscard]] auto Chunks() const -> const std::vector<std::vector<std::vector<int>>>&;
    [[nodiscard]] auto Dictionary() const -> const std::shared_ptr<const ZH_Dictionary>&;
    static auto get_combinations(const std::vector<std::vector<int>>& chunk)
        -> std::vector<std::vector<int>>;

    void SetAnnotationChoices(const std::map<CharacterSequence, Combination>& choices);
    void Reannotate();
    auto ContainsCharacterSequence(const CharacterSequence& charSeq) -> bool;

private:
    void annotate();
    std::shared_ptr<const ZH_Dictionary> dictionary;
    std::string annotated_text;
    std::vector<Item> items;
    std::vector<std::vector<std::vector<int>>> chunks;
    std::vector<std::vector<ZH_dicItemVec>> candidates;
    std::map<CharacterSequence, Combination> choices;
};
