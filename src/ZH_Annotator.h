#pragma once

#include <utils/StringU8.h>
#include <QSharedPointer>
#include <compare>
#include <list>
#include <memory>
#include <optional>
#include <set>
#include <span>
#include <vector>
#include "ZH_Dictionary.h"

class ZH_Annotator {
public:
    using ZH_dicItemVec = std::vector<ZH_Dictionary::Item>;

    ZH_Annotator(const utl::StringU8& _text, const std::shared_ptr<ZH_Dictionary>& _dictionary);
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

private:
    void annotate();
    const utl::StringU8 text;
    std::string annotated_text;
    const std::shared_ptr<ZH_Dictionary> dictionary;
    std::vector<Item> items;
    std::vector<std::vector<std::vector<int>>> chunks;
    std::vector<std::vector<ZH_dicItemVec>> candidates;
};
