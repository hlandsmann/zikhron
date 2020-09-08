#include "Cedict.h"
#include <fstream>
#include <iostream>
using std::cout;
using std::endl;
std::string Cedict::str(const icu::UnicodeString &myString) {
    std::string tempString;
    myString.toUTF8String(tempString);
    return tempString;
}

icu::UnicodeString Cedict::getPron(const icu::UnicodeString &pron) {
    std::vector<icu::UnicodeString> syllables;
    int                             startPos = 0;
    icu::UnicodeString              delim    = " ";
    icu::UnicodeString              result   = "";
    while (true) {
        auto syllable = getStrTo(pron, delim, startPos);
        if (syllable.isEmpty())
            break;
        syllables.push_back(syllable);
    }

    for (const auto &syllable : syllables)
        result += syllable.tempSubString(0, syllable.length() -1);
    return result;
}

bool Cedict::decodeLine(const std::string &line) {
    if (line.empty() || line[0] == '#')
        return false;

    icu::UnicodeString line_utf8 = line.c_str();
    int                startPos  = 0;
    icu::UnicodeString delim     = " ";

    // auto str1 = getStrTo(line_utf8, delim, startPos);
    // // if (startPos > 20)
    // //     cout << line << "\n asdf asdf asdf asdf asdf \n";
    // auto str2 = getStrTo(line_utf8, delim, startPos);

    // cout << str(getStrTo(line_utf8, delim, startPos)) << " : ";
    // cout << str(getStrTo(line_utf8, delim, startPos)) << "\n";

    auto trad = getStrTo(line_utf8, delim, startPos);
    // cout << str(trad) << " s: " << startPos << " ";
    auto simpl = getStrTo(line_utf8, delim, startPos);
    // cout << str(simpl) << " s: " << startPos << " ";
    dicKeys.push_back(DicKey{.simpl = simpl, .trad = trad});
    icu::UnicodeString delimBegin = "[";
    icu::UnicodeString delimEnd   = "]";

    startPos      = 0;
    auto pronPrem = getStrEnclosed(line_utf8, delimBegin, delimEnd, startPos);
    auto pron     = getPron(pronPrem);
    // cout << " : " << str(pron);
    std::vector<icu::UnicodeString> mean;
    icu::UnicodeString              delimMean = "/";

    while (true) {
        auto m = getStrTo(line_utf8, delimMean, startPos);
        if (m.isEmpty())
            break;
        mean.push_back(m);
        // cout << ", " << str(m);
    }
    // cout << "\n";
    dicValues.push_back(DicValue{.pron = pron, .mean = mean});
    return true;
}
icu::UnicodeString Cedict::getStrTo(const icu::UnicodeString &line,
                                    const icu::UnicodeString &delim,
                                    int &                     startPos) {
    int pos = line.indexOf(delim, startPos);
    // cout << " - " << pos << "|" << startPos << " - ";
    const auto result = line.tempSubString(startPos, pos - startPos);
    startPos          = pos + 1;
    return result;
}
icu::UnicodeString Cedict::getStrEnclosed(const icu::UnicodeString &line,
                                          const icu::UnicodeString &delimBegin,
                                          const icu::UnicodeString &delimEnd,
                                          int &                     startPos) {
    // cout << "si: " << startPos << " " << str(delimBegin) << "l:" << delimBegin.length() << " ";
    int posBegin = line.indexOf(delimBegin, startPos);
    int posEnd   = line.indexOf(delimEnd, startPos);
    // cout << "pb: " << posBegin << " pe: " << posEnd << " ";

    //  cout << " - " << posBegin << "|" << posEnd << " - " << startPos;
    const auto result = line.tempSubString(posBegin + 1, posEnd - posBegin - 1);
    startPos          = posEnd + 1;
    return result;
}

Cedict::Cedict(std::string fileName) {
    cout << fileName;
    int counter = 0;

    std::ifstream input(fileName);
    std::string   line;
    int           comments = 0, entries = 0;
    while (getline(input, line)) {
        if (decodeLine(line))
            entries++;
        else
            comments++;
    }
    cout << "Counted lines: " << counter << "\n";
    cout << "Comments: " << comments << " Entries: " << entries << "\n";

    for (const auto &dicKey : dicKeys) {
        counter++;

        if (0 == counter % 10000)
            cout << str(dicKey.simpl) << " : " << str(dicKey.trad) << endl;
    }
}
