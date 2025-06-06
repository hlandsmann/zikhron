#pragma once
#include "Token.h"

#include <spdlog/common.h>
#include <spdlog/logger.h>

#include <memory>
#include <string>
#include <vector>

namespace annotation {
class MecabWrapper;

enum class POS_Japanese {
    verb,
    adjective,
    noun,
    particle
};

struct MecabToken
{
    std::string surface;
    // Part of speech fields. The earlier fields are more general, the later fields are more specific.
    std::string pos1;
    std::string pos2;
    std::string pos3;
    std::string pos4;
    // - **cType:** 活用型, conjugation type. Will have a value like `五段-ラ行`.
    std::string conjugationType;
    // - **cForm:** 活用形, conjugation shape. Will have a value like `連用形-促音便`.
    std::string conjugationShape;
    // - **lForm:** 語彙素読み, lemma reading. The reading of the lemma in katakana,
    // this uses the same format as the `kana` field, not `pron`.
    std::string lemmaReading;
    // - **lemma:** 語彙素（＋語彙素細分類）. The lemma is a non-inflected "dictionary form" of a word.
    // UniDic lemmas sometimes include extra info or have unusual forms, like using katakana for some place names.
    std::string lemma;
    // - **orth:** 書字形出現形, the word as it appears in text,
    // this appears to be identical to the surface.
    std::string orth;
    // - **pron:** 発音形出現形, pronunciation. This is similar to kana except that long vowels are indicated with a ー,
    // so 講師 is こーし.
    std::string pronounciation;
    // - **orthBase:** 書字形基本形, the uninflected form of the word using its current written form. For example,
    // for 彷徨った the lemma is さ迷う but the orthBase is 彷徨う.
    std::string orthBase;
    // - **pronBase:** 発音形基本形, the pronunciation of the base form.
    // Like `pron` for the `lemma` or `orthBase`.
    std::string pronounciationBase;
    // - **goshu:** 語種, word type. Etymological category.
    // In order of frequency, 和, 固, 漢, 外, 混, 記号, 不明. Defined for all dictionary words, blank for unks.
    std::string goshu;
    // - **iType:** 語頭変化化型, "i" is for "initial".
    // This is the type of initial transformation the word undergoes when combining,
    // for example 兵 is へ半濁 because it can be read as べい in combination. This is available for <2% of entries.
    std::string iType;
    // - **iForm:** 語頭変化形, this is the initial form of the word in context, such as 基本形 or 半濁音形.
    std::string iForm;
    // - **fType:** 語末変化化型, "f" is for "final", but otherwise as iType.
    // For example 医学 is ク促 because it can change to いがっ (apparently). This is available for <0.1% of entries.
    std::string fType;
    // - **fForm:** 語末変化形, as iForm but for final transformations.
    std::string fForm;
    // - **iConType:** 語頭変化結合型, initial change fusion type.
    // Describes phonetic change at the start of the word in counting expressions.
    // Only available for a few hundred entries, mostly numbers. Values are N followed by a letter or number;
    // most entries with this value are numeric.
    std::string iConType;
    // - **fConType:** 語末変化結合型, final change fusion type. This is also used for counting expressions,
    // and like iConType it is only available for a few hundred entries. Unlike iConType the values are very complicated,
    // like `B1S6SjShS,B1S6S8SjShS`.
    std::string fConType;
    // - **type:** Appears to refer to the type of the lemma. See the details below for an overview.
    std::string lemmaType;
};

class Mecab
{
public:
    Mecab();
    auto split(const std::string& text) -> std::vector<MecabToken>;

    void setDebugSink(spdlog::sink_ptr sink);

private:
    std::shared_ptr<MecabWrapper> mecabWrapper;
    std::unique_ptr<spdlog::logger> log;
};
} // namespace annotation
