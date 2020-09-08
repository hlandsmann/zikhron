#ifndef ZH_ANNOTATOR_H
#define ZH_ANNOTATOR_H

#include <utils/StringU8.h>
#include <QSharedPointer>
#include <memory>
#include <vector>
#include "ZH_Dictionary.h"

class ZH_Annotator {
public:
    ZH_Annotator(const utl::StringU8& _text, const QSharedPointer<ZH_Dictionary>& _dictionary)
        : text(_text), dictionary(_dictionary) {
        annotate();
    };
    std::string Annotated() { return annotated_text; }

private:
    void annotate();
    const utl::StringU8 text;
    std::string annotated_text;
    const QSharedPointer<ZH_Dictionary> dictionary;
    std::vector<utl::StringU8> items;
    std::vector<std::vector<std::vector<int>>> chunks;
};
#endif /* ZH_ANNOTATOR_H */
