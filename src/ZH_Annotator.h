#ifndef ZH_ANNOTATOR_H
#define ZH_ANNOTATOR_H

#include <utils/StringU8.h>
#include <QSharedPointer>
#include <list>
#include <memory>
#include <optional>
#include <span>
#include <vector>
#include "ZH_Dictionary.h"

class ZH_Annotator {
public:
    using ZH_dicItemVec = std::vector<ZH_Dictionary::Item>;

    ZH_Annotator(const utl::StringU8& _text, const QSharedPointer<ZH_Dictionary>& _dictionary);
    auto Annotated() const -> const std::string&;

    struct Item {
        template <class string_t> Item(const string_t& _text) : text(_text), dicItemVec{} {}
        Item(const utl::StringU8& _text, const ZH_dicItemVec&& _dicItem)
            : text(_text), dicItemVec(_dicItem) {}
        utl::StringU8 text;
        ZH_dicItemVec dicItemVec;
    };
    auto Items() const -> const std::vector<Item>&;
    auto Candidates() const -> const std::vector<std::vector<ZH_dicItemVec>>&;

private:
    void annotate();
    const utl::StringU8 text;
    std::string annotated_text;
    const QSharedPointer<ZH_Dictionary> dictionary;
    std::vector<Item> items;
    std::vector<std::vector<std::vector<int>>> chunks;
    std::vector<std::vector<ZH_dicItemVec>> candidates;
};
#endif /* ZH_ANNOTATOR_H */
