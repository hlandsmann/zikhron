#pragma once

#include <unicode/unistr.h>
#include <string>
#include <string_view>
#include <vector>

class Cedict {
public:
    struct DicKey {
        icu::UnicodeString simpl;
        icu::UnicodeString trad;
    };

    struct DicValue {
        icu::UnicodeString              pron;
        std::vector<icu::UnicodeString> mean;
    };

    Cedict(const std::string fileName);
    ~Cedict() = default;

private:
    bool        decodeLine(const std::string &line);
    std::string str(const icu::UnicodeString &myString);

    icu::UnicodeString getStrTo(const icu::UnicodeString &line,
                                const icu::UnicodeString &delim,
                                int &                     startPos);
    icu::UnicodeString getStrEnclosed(const icu::UnicodeString &line,
                                      const icu::UnicodeString &delimBegin,
                                      const icu::UnicodeString &delimEnd,
                                      int &                     startPos);
    icu::UnicodeString getPron(const icu::UnicodeString &pron);

    std::vector<DicKey>   dicKeys;
    std::vector<DicValue> dicValues;
};
