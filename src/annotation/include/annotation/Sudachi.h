#pragma once
#include "Token.h"

#include <utils/ProcessPipe.h>

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace annotation {
struct SudachiToken
{
    std::string surface_merged;
    std::string surface;
    std::string pos1;
    std::string pos2;
    std::string pos3;
    std::string pos4;
    std::string pos5;
    std::string pos6;
    std::string normalized_form;
    std::string dictionary_form;
    std::string reading;
    int dictionary_id{};
    std::string extra_info;
};

class Sudachi
{
public:
    Sudachi(std::shared_ptr<utl::ProcessPipe> processPipe);
    [[nodiscard]] auto split(const std::string& text) const -> std::vector<SudachiToken>;
    [[nodiscard]] static auto mergeConjugation(const std::vector<SudachiToken>& sudachiTokens) -> std::vector<SudachiToken>;

private:
    [[nodiscard]] static auto shouldMerge(const SudachiToken& prev, const SudachiToken& curr) -> bool;
    [[nodiscard]] static auto parseLine(std::string_view line) -> SudachiToken;
    [[nodiscard]] static auto isCovered(const std::vector<SudachiToken>&, const std::string& text) -> bool;
    std::shared_ptr<utl::ProcessPipe> processPipe;
};
} // namespace annotation

// -----------------------------------------------------------
//  Inflection FORM → English gloss
//  (活用形)
// -----------------------------------------------------------
static const std::unordered_map<std::string, std::string> InflectionFormMeanings = {
        {"基本形", "dictionary/base form"},
        {"未然形", "imperfective form (before negation or volitional)"},
        {"未然形-サ", "imperfective form (s-stem variant)"},
        {"未然形-一般", "standard imperfective form"},
        {"未然形-縮約", "contracted imperfective form"},
        {"連用形", "continuative form (before auxiliaries such as ます・た・て)"},
        {"連用形-一般", "standard continuative form"},
        {"連用形-促音便", "continuative form with gemination (small っ)"},
        {"連用形-ウ音便", "continuative form with u-sound change"},
        {"連用形-イ音便", "continuative form with i-sound change"},
        {"連用形-特殊", "special continuative form"},
        {"連体形", "attributive form (before nouns)"},
        {"連体形-一般", "standard attributive form"},
        {"終止形", "terminal form (sentence-ending)"},
        {"終止形-一般", "standard terminal form"},
        {"仮定形", "conditional form (if-clause)"},
        {"仮定形-一般", "standard conditional form"},
        {"仮定形-特殊", "special conditional form"},
        {"命令形", "imperative form"},
        {"命令形-一般", "standard imperative form"},
        {"命令形-特殊", "special imperative form"},
        {"語幹", "word stem (used for adjectives and irregular verbs)"},
        {"語幹-一般", "standard stem form"},
        {"体言接続", "nominal connective form"},
        {"基本形-縮約", "contracted base form"},
        {"仮定形-縮約", "contracted conditional form"},
        {"命令ｅ形", "imperative e-form (polite imperative)"},
        {"命令ｒｏ形", "imperative ro-form"},
        {"仮定縮約", "conditional contracted form"},
        {"無変化", "non-inflecting form"}};

// -----------------------------------------------------------
//  Inflection TYPE → English gloss
//  (活用型)
// -----------------------------------------------------------
static const std::unordered_map<std::string, std::string> InflectionTypeMeanings = {
        {"五段-カ行", "Godan verb (k-row)"},
        {"五段-ガ行", "Godan verb (g-row)"},
        {"五段-サ行", "Godan verb (s-row)"},
        {"五段-タ行", "Godan verb (t-row)"},
        {"五段-ナ行", "Godan verb (n-row)"},
        {"五段-バ行", "Godan verb (b-row)"},
        {"五段-マ行", "Godan verb (m-row)"},
        {"五段-ラ行", "Godan verb (r-row)"},
        {"五段-ワ行", "Godan verb (w-row)"},
        {"上一段-ア行", "upper-one-row (a-line) Ichidan verb"},
        {"上一段-カ行", "upper-one-row (ka-line) Ichidan verb"},
        {"上一段-ガ行", "upper-one-row (ga-line) Ichidan verb"},
        {"上一段-サ行", "upper-one-row (sa-line) Ichidan verb"},
        {"上一段-タ行", "upper-one-row (ta-line) Ichidan verb"},
        {"上一段-ナ行", "upper-one-row (na-line) Ichidan verb"},
        {"上一段-ハ行", "upper-one-row (ha-line) Ichidan verb"},
        {"上一段-バ行", "upper-one-row (ba-line) Ichidan verb"},
        {"上一段-マ行", "upper-one-row (ma-line) Ichidan verb"},
        {"上一段-ラ行", "upper-one-row (ra-line) Ichidan verb"},
        {"下一段-カ行", "lower-one-row (ka-line) Ichidan verb"},
        {"下一段-ガ行", "lower-one-row (ga-line) Ichidan verb"},
        {"下一段-サ行", "lower-one-row (sa-line) Ichidan verb"},
        {"下一段-タ行", "lower-one-row (ta-line) Ichidan verb"},
        {"下一段-ナ行", "lower-one-row (na-line) Ichidan verb"},
        {"下一段-バ行", "lower-one-row (ba-line) Ichidan verb"},
        {"下一段-マ行", "lower-one-row (ma-line) Ichidan verb"},
        {"下一段-ラ行", "lower-one-row (ra-line) Ichidan verb"},
        {"一段", "Ichidan (ru-verb)"},
        {"サ変", "suru-irregular verb"},
        {"カ変", "kuru-irregular verb"},
        {"ザ変", "zuru-variant of suru"},
        {"ラ変", "ra-irregular verb"},
        {"形容詞-イ段", "i-adjective"},
        {"形容詞-ナノ", "na-adjective"},
        {"形容詞-ト", "to-adjectival form"},
        {"助動詞-タ", "auxiliary ‘ta’ (past tense)"},
        {"助動詞-ナイ", "auxiliary ‘nai’ (negation)"},
        {"助動詞-ダ", "auxiliary/copula ‘da’"},
        {"助動詞-マス", "auxiliary ‘masu’ (polite)"},
        {"助動詞-ラレル", "auxiliary ‘rareru’ (passive/potential)"},
        {"助動詞-レル", "auxiliary ‘reru’ (passive/potential)"},
        {"助動詞-ヨウ", "auxiliary ‘you’ (volitional)"},
        {"助動詞-ベシ", "auxiliary ‘beshi’ (archaic modal)"},
        {"特殊-デス", "special polite copula ‘desu’"},
        {"特殊-ダッ", "special past copula stem"},
        {"特殊-ナ", "special adjective-linking ‘na’"},
        {"特殊-ヌ", "archaic negative ‘nu’"},
        {"特殊-タリ", "archaic copular ‘tari’"},
        {"特殊-ナリ", "archaic copular ‘nari’"},
        {"特殊-マス", "special polite stem"},
        {"不変化型", "non-inflecting word (nouns, particles, etc.)"}};
