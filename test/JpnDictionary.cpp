#include "JpnDictionary.h"

#include <spdlog/spdlog.h>

#include <cstddef>
#include <filesystem>
#include <magic_enum.hpp>
#include <map>
#include <pugixml.hpp>
#include <string>
#include <string_view>
#include <vector>
using namespace std::literals;

namespace japaneseDic {
struct POS
{
    std::map<std::string, PartOfSpeech> data = {
            {"adj-f", PartOfSpeech::adjective},     // noun or verb acting prenominally
            {"adj-i", PartOfSpeech::adjective},     // adjective (keiyoushi)
            {"adj-ix", PartOfSpeech::adjective},    // adjective (keiyoushi) - yoi/ii class
            {"adj-kari", PartOfSpeech::adjective},  // 'kari' adjective (archaic)
            {"adj-ku", PartOfSpeech::adjective},    // 'ku' adjective (archaic)
            {"adj-na", PartOfSpeech::adjective},    // adjectival nouns or quasi-adjectives (keiyodoshi)
            {"adj-nari", PartOfSpeech::adjective},  // archaic/formal form of na-adjective
            {"adj-no", PartOfSpeech::adjective},    // nouns which may take the genitive case particle 'no'
            {"adj-pn", PartOfSpeech::adjective},    // pre-noun adjectival (rentaishi)
            {"adj-shiku", PartOfSpeech::adjective}, // 'shiku' adjective (archaic)
            {"adj-t", PartOfSpeech::adjective},     // 'taru' adjective
            {"adv", PartOfSpeech::adverb},          // adverb (fukushi)
            {"adv-to", PartOfSpeech::adverb},       // adverb taking the 'to' particle
            {"aux", PartOfSpeech::undefined},       // auxiliary
            {"aux-adj", PartOfSpeech::adjective},   // auxiliary adjective
            {"aux-v", PartOfSpeech::verb},          // auxiliary verb
            {"conj", PartOfSpeech::conjunction},    // conjunction
            {"cop", PartOfSpeech::undefined},       // copula
            {"ctr", PartOfSpeech::undefined},       // counter
            {"exp", PartOfSpeech::adjective},       // expressions (phrases, clauses, etc.)
            {"int", PartOfSpeech::adjective},       // interjection (kandoushi)
            {"n", PartOfSpeech::noun},              // noun (common) (futsuumeishi)
            {"n-adv", PartOfSpeech::noun},          // adverbial noun (fukushitekimeishi)
            {"n-pr", PartOfSpeech::noun},           // proper noun
            {"n-pref", PartOfSpeech::noun},         // noun, used as a prefix
            {"n-suf", PartOfSpeech::noun},          // noun, used as a suffix
            {"n-t", PartOfSpeech::noun},            // noun (temporal) (jisoumeishi)
            {"num", PartOfSpeech::undefined},       // numeric
            {"pn", PartOfSpeech::pronoun},          // pronoun
            {"pref", PartOfSpeech::prefix},         // prefix
            {"prt", PartOfSpeech::particle},        // particle
            {"suf", PartOfSpeech::suffix},          // suffix
            {"unc", PartOfSpeech::undefined},       // unclassified
            {"v-unspec", PartOfSpeech::verb},       // verb unspecified
            {"v1", PartOfSpeech::verb},             // Ichidan verb
            {"v1-s", PartOfSpeech::verb},           // Ichidan verb - kureru special class
            {"v2a-s", PartOfSpeech::verb},          // Nidan verb with 'u' ending (archaic)
            {"v2b-k", PartOfSpeech::verb},          // Nidan verb (upper class) with 'bu' ending (archaic)
            {"v2b-s", PartOfSpeech::verb},          // Nidan verb (lower class) with 'bu' ending (archaic)
            {"v2d-k", PartOfSpeech::verb},          // Nidan verb (upper class) with 'dzu' ending (archaic)
            {"v2d-s", PartOfSpeech::verb},          // Nidan verb (lower class) with 'dzu' ending (archaic)
            {"v2g-k", PartOfSpeech::verb},          // Nidan verb (upper class) with 'gu' ending (archaic)
            {"v2g-s", PartOfSpeech::verb},          // Nidan verb (lower class) with 'gu' ending (archaic)
            {"v2h-k", PartOfSpeech::verb},          // Nidan verb (upper class) with 'hu/fu' ending (archaic)
            {"v2h-s", PartOfSpeech::verb},          // Nidan verb (lower class) with 'hu/fu' ending (archaic)
            {"v2k-k", PartOfSpeech::verb},          // Nidan verb (upper class) with 'ku' ending (archaic)
            {"v2k-s", PartOfSpeech::verb},          // Nidan verb (lower class) with 'ku' ending (archaic)
            {"v2m-k", PartOfSpeech::verb},          // Nidan verb (upper class) with 'mu' ending (archaic)
            {"v2m-s", PartOfSpeech::verb},          // Nidan verb (lower class) with 'mu' ending (archaic)
            {"v2n-s", PartOfSpeech::verb},          // Nidan verb (lower class) with 'nu' ending (archaic)
            {"v2r-k", PartOfSpeech::verb},          // Nidan verb (upper class) with 'ru' ending (archaic)
            {"v2r-s", PartOfSpeech::verb},          // Nidan verb (lower class) with 'ru' ending (archaic)
            {"v2s-s", PartOfSpeech::verb},          // Nidan verb (lower class) with 'su' ending (archaic)
            {"v2t-k", PartOfSpeech::verb},          // Nidan verb (upper class) with 'tsu' ending (archaic)
            {"v2t-s", PartOfSpeech::verb},          // Nidan verb (lower class) with 'tsu' ending (archaic)
            {"v2w-s", PartOfSpeech::verb},          // Nidan verb (lower class) with 'u' ending and 'we' conjugation (archaic)
            {"v2y-k", PartOfSpeech::verb},          // Nidan verb (upper class) with 'yu' ending (archaic)
            {"v2y-s", PartOfSpeech::verb},          // Nidan verb (lower class) with 'yu' ending (archaic)
            {"v2z-s", PartOfSpeech::verb},          // Nidan verb (lower class) with 'zu' ending (archaic)
            {"v4b", PartOfSpeech::verb},            // Yodan verb with 'bu' ending (archaic)
            {"v4g", PartOfSpeech::verb},            // Yodan verb with 'gu' ending (archaic)
            {"v4h", PartOfSpeech::verb},            // Yodan verb with 'hu/fu' ending (archaic)
            {"v4k", PartOfSpeech::verb},            // Yodan verb with 'ku' ending (archaic)
            {"v4m", PartOfSpeech::verb},            // Yodan verb with 'mu' ending (archaic)
            {"v4n", PartOfSpeech::verb},            // Yodan verb with 'nu' ending (archaic)
            {"v4r", PartOfSpeech::verb},            // Yodan verb with 'ru' ending (archaic)
            {"v4s", PartOfSpeech::verb},            // Yodan verb with 'su' ending (archaic)
            {"v4t", PartOfSpeech::verb},            // Yodan verb with 'tsu' ending (archaic)
            {"v5aru", PartOfSpeech::verb},          // Godan verb - -aru special class
            {"v5b", PartOfSpeech::verb},            // Godan verb with 'bu' ending
            {"v5g", PartOfSpeech::verb},            // Godan verb with 'gu' ending
            {"v5k", PartOfSpeech::verb},            // Godan verb with 'ku' ending
            {"v5k-s", PartOfSpeech::verb},          // Godan verb - Iku/Yuku special class
            {"v5m", PartOfSpeech::verb},            // Godan verb with 'mu' ending
            {"v5n", PartOfSpeech::verb},            // Godan verb with 'nu' ending
            {"v5r", PartOfSpeech::verb},            // Godan verb with 'ru' ending
            {"v5r-i", PartOfSpeech::verb},          // Godan verb with 'ru' ending (irregular verb)
            {"v5s", PartOfSpeech::verb},            // Godan verb with 'su' ending
            {"v5t", PartOfSpeech::verb},            // Godan verb with 'tsu' ending
            {"v5u", PartOfSpeech::verb},            // Godan verb with 'u' ending
            {"v5u-s", PartOfSpeech::verb},          // Godan verb with 'u' ending (special class)
            {"v5uru", PartOfSpeech::verb},          // Godan verb - Uru old class verb (old form of Eru)
            {"vi", PartOfSpeech::verb},             // intransitive verb
            {"vk", PartOfSpeech::verb},             // Kuru verb - special class
            {"vn", PartOfSpeech::verb},             // irregular nu verb
            {"vr", PartOfSpeech::verb},             // irregular ru verb, plain form ends with -ri
            {"vs", PartOfSpeech::verb},             // noun or participle which takes the aux. verb suru
            {"vs-c", PartOfSpeech::verb},           // su verb - precursor to the modern suru
            {"vs-i", PartOfSpeech::verb},           // suru verb - included
            {"vs-s", PartOfSpeech::verb},           // suru verb - special class
            {"vt", PartOfSpeech::verb},             // transitive verb
            {"vz", PartOfSpeech::verb},             // Ichidan verb - zuru verb (alternative form of -jiru verbs)
    };
};

struct KanjiElement
{
    constexpr static std::string_view s_base = "k_ele";
    constexpr static std::string_view s_key = "keb";
    constexpr static std::string_view s_info = "ke_inf";
    constexpr static std::string_view s_priority = "ke_pri";
};

struct ReadingElement
{
    constexpr static std::string_view s_base = "r_ele";
    constexpr static std::string_view s_key = "reb";
    constexpr static std::string_view s_noKanji = "re_nokanji"; // mostly empty, ignore?
    constexpr static std::string_view s_restrict = "re_restr";
    constexpr static std::string_view s_info = "re_inf";
    constexpr static std::string_view s_priority = "re_pri";
};

struct Sense
{
    constexpr static std::string_view s_base = "sense";
    constexpr static std::string_view s_restrictKanji = "stagk";
    constexpr static std::string_view s_restrictReading = "stagr";
    constexpr static std::string_view s_crossReference = "xref";
    constexpr static std::string_view s_antonym = "ant";
    constexpr static std::string_view s_partOfSpeech = "pos"; // if not present in sense, use pos of previous sense
    constexpr static std::string_view s_field = "field";
    constexpr static std::string_view s_misc = "misc"; // if not present in sense, use misc of previous sense
    constexpr static std::string_view s_info = "s_inf";
    // constexpr static std::string_view loanword = "lsource";
    constexpr static std::string_view s_dialect = "dial";
    constexpr static std::string_view s_meaning = "gloss";
};

auto getSize(const pugi::xml_node& node) -> std::size_t
{
    std::size_t size = 0;
    for (const auto& _ : node.children()) {
        size++;
    }
    return size;
}

auto parseSense(const pugi::xml_node& node)
{
    std::vector<std::string> meanings;
    std::vector<std::string> restrictKanji;
    std::vector<std::string> restrictReading;
    std::vector<std::string> crossReference;
    std::vector<std::string> antonym;
    std::vector<std::string> partOfSpeech; // if not present in sense, use pos of previous sense
    std::vector<std::string> field;
    std::vector<std::string> misc; // if not present in sense, use misc of previous sense
    std::vector<std::string> dialect;
    std::string info;
    for (const auto& snode : node.children()) {
        if (snode.name() == Sense::s_meaning) {
            meanings.emplace_back(snode.child_value());
        }
        if (snode.name() == Sense::s_restrictKanji) {
            restrictKanji.emplace_back(snode.child_value());
        }
        if (snode.name() == Sense::s_restrictReading) {
            restrictReading.emplace_back(snode.child_value());
        }
        if (snode.name() == Sense::s_crossReference) {
            crossReference.emplace_back(snode.child_value());
        }
        if (snode.name() == Sense::s_antonym) {
            antonym.emplace_back(snode.child_value());
        }
        if (snode.name() == Sense::s_partOfSpeech) {
            partOfSpeech.emplace_back(snode.child_value());
        }
        if (snode.name() == Sense::s_field) {
            field.emplace_back(snode.child_value());
        }
        if (snode.name() == Sense::s_misc) {
            misc.emplace_back(snode.child_value());
        }
        if (snode.name() == Sense::s_dialect) {
            dialect.emplace_back(snode.child_value());
        }
        if (snode.name() == Sense::s_info) {
            info=snode.child_value();
        }
    }
    // spdlog::info("key: {}", key);
    if (dialect.size() > 1) {
        spdlog::info("info: {}", fmt::join(dialect, " -::- "));
    }
}

auto parseKanjiElement(const pugi::xml_node& node)
{
    std::string key; // only one key per reading element
    std::vector<std::string> infos;
    std::vector<std::string> priorities;
    for (const auto& snode : node.children()) {
        if (snode.name() == KanjiElement::s_key) {
            key = snode.child_value();
            continue;
        }
        if (snode.name() == KanjiElement::s_info) {
            // two elements have multiple info values
            infos.emplace_back(snode.child_value());
            continue;
        }
        if (snode.name() == KanjiElement::s_priority) {
            priorities.emplace_back(snode.child_value());
            continue;
        }
    }
    // spdlog::info("key: {}", key);
    // if (infos.size() > 1) {
    //     spdlog::info("info: {}", fmt::join(infos, ", "));
    // }
}

auto parseReadingElement(const pugi::xml_node& node)
{
    std::string key; // only one key per reading element
    std::vector<std::string> infos;
    std::vector<std::string> restricts;
    std::vector<std::string> priorities;
    for (const auto& snode : node.children()) {
        if (snode.name() == ReadingElement::s_key) {
            key = snode.child_value();
            continue;
        }
        if (snode.name() == ReadingElement::s_restrict) {
            restricts.emplace_back(snode.child_value());
            continue;
        }
        if (snode.name() == ReadingElement::s_info) {
            // two elements have multiple info values
            infos.emplace_back(snode.child_value());
            continue;
        }
        if (snode.name() == ReadingElement::s_priority) {
            priorities.emplace_back(snode.child_value());
            continue;
        }
    }
    // spdlog::info("key: {}", key);
    // if (infos.size() > 1) {
    //     spdlog::info("info: {}", fmt::join(infos, ", "));
    // }
}

void parseEntry(const pugi::xml_node& entry)
{
    // spdlog::info("eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee");
    std::size_t r_ele = 0;
    std::size_t sense = 0;
    for (const auto& node : entry.children()) {
        // spdlog::info("name: {}: {} ", a.name(), a.value());
        if (node.name() == KanjiElement::s_base) {
            parseKanjiElement(node);
        }
        if (node.name() == ReadingElement::s_base) {
            parseReadingElement(node);
        }
        if (node.name() == Sense::s_base) {
            parseSense(node);
        }

        // if (a.name() == "sense"sv) {
        //     sense++;
        // }
        // if (!a.children().empty()) {
        //     size_t size = 0;
        //     for (const auto& b : a.children()) {
        //         size++;
        //         // spdlog::info("    name: {}: {} ", b.name(), b.value());
        //         if (b.name() == "reb"sv) {
        //             // spdlog::info("       a: {} ", b.child_value());
        //             if (getSize(b) != 1 || r_ele != 1) {
        //                 spdlog::info("       a: {} ", b.child_value());
        //                 spdlog::info("----------sreb: {} ", r_ele);
        //             }
        //         }
        //         // if (b.name() == "re_inf"sv && b.child_value() != ""sv) {
        //         //     spdlog::info("       ank: {} ", b.child_value());
        //         // }
        //         if (b.name() == "gloss"sv) {
        //             spdlog::info("       a: {} ", b.child_value());
        //             if (getSize(b) != 1 || sense != 1) {
        //                 spdlog::info("----------sgloss: {}, s: {} ", getSize(b), size);
        //             }
        //         }
        //     }
        // }
    }
}

JpnDictionary::JpnDictionary(const std::filesystem::path& xmlFile)
{
    pugi::xml_document doc;

    pugi::xml_parse_result result = doc.load_file(xmlFile.c_str());

    spdlog::info("Loading japanese dictioary: {}", result.description());
    pugi::xml_node jmDict = doc.child("JMdict");

    int count = 0;
    for (const pugi::xml_node& entry : jmDict.children("entry")) {
        // if (count > 1000 && count < 1010) {
        parseEntry(entry);
        // }
        count++;
        // if (count > 10) {
        //     break;
        // }
        count++;
    }
    spdlog::info("found {} entries", count);
}
} // namespace japaneseDic
