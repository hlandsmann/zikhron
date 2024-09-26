#include "JpnDictionary.h"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <filesystem>
#include <generator>
#include <iterator>
#include <magic_enum.hpp>
#include <map>
#include <pugixml.hpp>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>
using namespace std::literals;
namespace ranges = std::ranges;
namespace views = std::views;

namespace japaneseDic {
constexpr std::string dot = "・";

struct POS
{
    std::map<std::string, PartOfSpeech> data = {
            {"&adj-f;", PartOfSpeech::adjective},     // noun or verb acting prenominally
            {"&adj-i;", PartOfSpeech::adjective},     // adjective (keiyoushi)
            {"&adj-ix;", PartOfSpeech::adjective},    // adjective (keiyoushi) - yoi/ii class
            {"&adj-kari;", PartOfSpeech::adjective},  // 'kari' adjective (archaic)
            {"&adj-ku;", PartOfSpeech::adjective},    // 'ku' adjective (archaic)
            {"&adj-na;", PartOfSpeech::adjective},    // adjectival nouns or quasi-adjectives (keiyodoshi)
            {"&adj-nari;", PartOfSpeech::adjective},  // archaic/formal form of na-adjective
            {"&adj-no;", PartOfSpeech::adjective},    // nouns which may take the genitive case particle 'no'
            {"&adj-pn;", PartOfSpeech::adjective},    // pre-noun adjectival (rentaishi)
            {"&adj-shiku;", PartOfSpeech::adjective}, // 'shiku' adjective (archaic)
            {"&adj-t;", PartOfSpeech::adjective},     // 'taru' adjective
            {"&adv;", PartOfSpeech::adverb},          // adverb (fukushi)
            {"&adv-to;", PartOfSpeech::adverb},       // adverb taking the 'to' particle
            {"&aux;", PartOfSpeech::undefined},       // auxiliary
            {"&aux-adj;", PartOfSpeech::adjective},   // auxiliary adjective
            {"&aux-v;", PartOfSpeech::verb},          // auxiliary verb
            {"&conj;", PartOfSpeech::conjunction},    // conjunction
            {"&cop;", PartOfSpeech::undefined},       // copula
            {"&ctr;", PartOfSpeech::undefined},       // counter
            {"&exp;", PartOfSpeech::adjective},       // expressions (phrases, clauses, etc.)
            {"&int;", PartOfSpeech::adjective},       // interjection (kandoushi)
            {"&n;", PartOfSpeech::noun},              // noun (common) (futsuumeishi)
            {"&n-adv;", PartOfSpeech::noun},          // adverbial noun (fukushitekimeishi)
            {"&n-pr;", PartOfSpeech::noun},           // proper noun
            {"&n-pref;", PartOfSpeech::noun},         // noun, used as a prefix
            {"&n-suf;", PartOfSpeech::noun},          // noun, used as a suffix
            {"&n-t;", PartOfSpeech::noun},            // noun (temporal) (jisoumeishi)
            {"&num;", PartOfSpeech::undefined},       // numeric
            {"&pn;", PartOfSpeech::pronoun},          // pronoun
            {"&pref;", PartOfSpeech::prefix},         // prefix
            {"&prt;", PartOfSpeech::particle},        // particle
            {"&suf;", PartOfSpeech::suffix},          // suffix
            {"&unc;", PartOfSpeech::undefined},       // unclassified
            {"&v-unspec;", PartOfSpeech::verb},       // verb unspecified
            {"&v1;", PartOfSpeech::verb},             // Ichidan verb
            {"&v1-s;", PartOfSpeech::verb},           // Ichidan verb - kureru special class
            {"&v2a-s;", PartOfSpeech::verb},          // Nidan verb with 'u' ending (archaic)
            {"&v2b-k;", PartOfSpeech::verb},          // Nidan verb (upper class) with 'bu' ending (archaic)
            {"&v2b-s;", PartOfSpeech::verb},          // Nidan verb (lower class) with 'bu' ending (archaic)
            {"&v2d-k;", PartOfSpeech::verb},          // Nidan verb (upper class) with 'dzu' ending (archaic)
            {"&v2d-s;", PartOfSpeech::verb},          // Nidan verb (lower class) with 'dzu' ending (archaic)
            {"&v2g-k;", PartOfSpeech::verb},          // Nidan verb (upper class) with 'gu' ending (archaic)
            {"&v2g-s;", PartOfSpeech::verb},          // Nidan verb (lower class) with 'gu' ending (archaic)
            {"&v2h-k;", PartOfSpeech::verb},          // Nidan verb (upper class) with 'hu/fu' ending (archaic)
            {"&v2h-s;", PartOfSpeech::verb},          // Nidan verb (lower class) with 'hu/fu' ending (archaic)
            {"&v2k-k;", PartOfSpeech::verb},          // Nidan verb (upper class) with 'ku' ending (archaic)
            {"&v2k-s;", PartOfSpeech::verb},          // Nidan verb (lower class) with 'ku' ending (archaic)
            {"&v2m-k;", PartOfSpeech::verb},          // Nidan verb (upper class) with 'mu' ending (archaic)
            {"&v2m-s;", PartOfSpeech::verb},          // Nidan verb (lower class) with 'mu' ending (archaic)
            {"&v2n-s;", PartOfSpeech::verb},          // Nidan verb (lower class) with 'nu' ending (archaic)
            {"&v2r-k;", PartOfSpeech::verb},          // Nidan verb (upper class) with 'ru' ending (archaic)
            {"&v2r-s;", PartOfSpeech::verb},          // Nidan verb (lower class) with 'ru' ending (archaic)
            {"&v2s-s;", PartOfSpeech::verb},          // Nidan verb (lower class) with 'su' ending (archaic)
            {"&v2t-k;", PartOfSpeech::verb},          // Nidan verb (upper class) with 'tsu' ending (archaic)
            {"&v2t-s;", PartOfSpeech::verb},          // Nidan verb (lower class) with 'tsu' ending (archaic)
            {"&v2w-s;", PartOfSpeech::verb},          // Nidan verb (lower class) with 'u' ending and 'we' conjugation (archaic)
            {"&v2y-k;", PartOfSpeech::verb},          // Nidan verb (upper class) with 'yu' ending (archaic)
            {"&v2y-s;", PartOfSpeech::verb},          // Nidan verb (lower class) with 'yu' ending (archaic)
            {"&v2z-s;", PartOfSpeech::verb},          // Nidan verb (lower class) with 'zu' ending (archaic)
            {"&v4b;", PartOfSpeech::verb},            // Yodan verb with 'bu' ending (archaic)
            {"&v4g;", PartOfSpeech::verb},            // Yodan verb with 'gu' ending (archaic)
            {"&v4h;", PartOfSpeech::verb},            // Yodan verb with 'hu/fu' ending (archaic)
            {"&v4k;", PartOfSpeech::verb},            // Yodan verb with 'ku' ending (archaic)
            {"&v4m;", PartOfSpeech::verb},            // Yodan verb with 'mu' ending (archaic)
            {"&v4n;", PartOfSpeech::verb},            // Yodan verb with 'nu' ending (archaic)
            {"&v4r;", PartOfSpeech::verb},            // Yodan verb with 'ru' ending (archaic)
            {"&v4s;", PartOfSpeech::verb},            // Yodan verb with 'su' ending (archaic)
            {"&v4t;", PartOfSpeech::verb},            // Yodan verb with 'tsu' ending (archaic)
            {"&v5aru;", PartOfSpeech::verb},          // Godan verb - -aru special class
            {"&v5b;", PartOfSpeech::verb},            // Godan verb with 'bu' ending
            {"&v5g;", PartOfSpeech::verb},            // Godan verb with 'gu' ending
            {"&v5k;", PartOfSpeech::verb},            // Godan verb with 'ku' ending
            {"&v5k-s;", PartOfSpeech::verb},          // Godan verb - Iku/Yuku special class
            {"&v5m;", PartOfSpeech::verb},            // Godan verb with 'mu' ending
            {"&v5n;", PartOfSpeech::verb},            // Godan verb with 'nu' ending
            {"&v5r;", PartOfSpeech::verb},            // Godan verb with 'ru' ending
            {"&v5r-i;", PartOfSpeech::verb},          // Godan verb with 'ru' ending (irregular verb)
            {"&v5s;", PartOfSpeech::verb},            // Godan verb with 'su' ending
            {"&v5t;", PartOfSpeech::verb},            // Godan verb with 'tsu' ending
            {"&v5u;", PartOfSpeech::verb},            // Godan verb with 'u' ending
            {"&v5u-s;", PartOfSpeech::verb},          // Godan verb with 'u' ending (special class)
            {"&v5uru;", PartOfSpeech::verb},          // Godan verb - Uru old class verb (old form of Eru)
            {"&vi;", PartOfSpeech::verb},             // intransitive verb
            {"&vk;", PartOfSpeech::verb},             // Kuru verb - special class
            {"&vn;", PartOfSpeech::verb},             // irregular nu verb
            {"&vr;", PartOfSpeech::verb},             // irregular ru verb, plain form ends with -ri
            {"&vs;", PartOfSpeech::verb},             // noun or participle which takes the aux. verb suru
            {"&vs-c;", PartOfSpeech::verb},           // su verb - precursor to the modern suru
            {"&vs-i;", PartOfSpeech::verb},           // suru verb - included
            {"&vs-s;", PartOfSpeech::verb},           // suru verb - special class
            {"&vt;", PartOfSpeech::verb},             // transitive verb
            {"&vz;", PartOfSpeech::verb},             // Ichidan verb - zuru verb (alternative form of -jiru verbs)
    };
};

auto parsePOS(const std::string& partOfSpeech) -> PartOfSpeech
{
    static POS pos;
    try {
        return pos.data.at(partOfSpeech);
    } catch (...) {
        spdlog::error("pos not found: {}", partOfSpeech);
        return PartOfSpeech::undefined;
    }
};

struct KanjiElement
{
    constexpr static std::string_view s_base = "k_ele";
    constexpr static std::string_view s_key = "keb";
    constexpr static std::string_view s_info = "ke_inf";
    constexpr static std::string_view s_priority = "ke_pri";

    std::string key;
    std::vector<std::string> infos;
    std::vector<std::string> priorities;
};

struct ReadingElement
{
    constexpr static std::string_view s_base = "r_ele";
    constexpr static std::string_view s_key = "reb";
    constexpr static std::string_view s_noKanji = "re_nokanji"; // mostly empty, ignore?
    constexpr static std::string_view s_restrict = "re_restr";
    constexpr static std::string_view s_info = "re_inf";
    constexpr static std::string_view s_priority = "re_pri";

    std::string key;
    std::vector<std::string> infos;
    std::vector<std::string> restricts;
    std::vector<std::string> priorities;
};

struct Sense
{
    constexpr static std::string_view s_base = "sense";
    constexpr static std::string_view s_restrictKanji = "stagk";
    constexpr static std::string_view s_restrictReading = "stagr";
    constexpr static std::string_view s_crossReference = "xref";
    constexpr static std::string_view s_antonym = "ant";
    constexpr static std::string_view s_partOfSpeech = "pos";
    constexpr static std::string_view s_field = "field";
    constexpr static std::string_view s_misc = "misc";
    constexpr static std::string_view s_info = "s_inf";
    // constexpr static std::string_view loanword = "lsource";
    constexpr static std::string_view s_dialect = "dial";
    constexpr static std::string_view s_meaning = "gloss";

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
};

[[nodiscard]] auto parseKanjiElement(const pugi::xml_node& node) -> KanjiElement
{
    KanjiElement kanjiElement;
    for (const auto& snode : node.children()) {
        if (snode.name() == KanjiElement::s_key) {
            assert(kanjiElement.key.empty());
            kanjiElement.key = snode.child_value();
            continue;
        }
        if (snode.name() == KanjiElement::s_info) {
            kanjiElement.infos.emplace_back(snode.child_value());
            continue;
        }
        if (snode.name() == KanjiElement::s_priority) {
            kanjiElement.priorities.emplace_back(snode.child_value());
            continue;
        }
    }
    // spdlog::info("key: {}", key);
    // if (infos.size() > 1) {
    //     spdlog::info("info: {}", fmt::join(infos, ", "));
    // }
    return kanjiElement;
}

[[nodiscard]] auto parseReadingElement(const pugi::xml_node& node) -> ReadingElement
{
    ReadingElement readingElement;
    for (const auto& snode : node.children()) {
        if (snode.name() == ReadingElement::s_key) {
            assert(readingElement.key.empty());
            readingElement.key = snode.child_value();
            continue;
        }
        if (snode.name() == ReadingElement::s_restrict) {
            readingElement.restricts.emplace_back(snode.child_value());
            continue;
        }
        if (snode.name() == ReadingElement::s_info) {
            readingElement.infos.emplace_back(snode.child_value());
            continue;
        }
        if (snode.name() == ReadingElement::s_priority) {
            readingElement.priorities.emplace_back(snode.child_value());
            continue;
        }
    }
    // spdlog::info("key: {}", key);
    // if (infos.size() > 1) {
    //     spdlog::info("info: {}", fmt::join(infos, ", "));
    // }
    return readingElement;
}

[[nodiscard]] auto parseSense(const pugi::xml_node& node) -> Sense
{
    Sense sense;
    for (const auto& snode : node.children()) {
        if (snode.name() == Sense::s_meaning) {
            sense.meanings.emplace_back(snode.child_value());
            continue;
        }
        if (snode.name() == Sense::s_restrictKanji) {
            sense.restrictKanji.emplace_back(snode.child_value());
            continue;
        }
        if (snode.name() == Sense::s_restrictReading) {
            sense.restrictReading.emplace_back(snode.child_value());
            continue;
        }
        if (snode.name() == Sense::s_crossReference) {
            sense.crossReference.emplace_back(snode.child_value());
            continue;
        }
        if (snode.name() == Sense::s_antonym) {
            sense.antonym.emplace_back(snode.child_value());
            continue;
        }
        if (snode.name() == Sense::s_partOfSpeech) {
            sense.partOfSpeech.emplace_back(snode.child_value());
            continue;
        }
        if (snode.name() == Sense::s_field) {
            sense.field.emplace_back(snode.child_value());
            continue;
        }
        if (snode.name() == Sense::s_misc) {
            sense.misc.emplace_back(snode.child_value());
            continue;
        }
        if (snode.name() == Sense::s_dialect) {
            sense.dialect.emplace_back(snode.child_value());
            continue;
        }
        if (snode.name() == Sense::s_info) {
            assert(sense.info.empty());
            sense.info = snode.child_value();
            continue;
        }
    }
    // spdlog::info("key: {}", key);
    // if (dialect.size() > 1) {
    //     spdlog::info("info: {}", fmt::join(dialect, " -::- "));
    // }
    assert(!sense.meanings.empty());
    return sense;
}

void spreadReadings(std::vector<Entry>& entries, const std::vector<ReadingElement>& readings)
{
    auto insertReading = [](Entry& entry, const ReadingElement& reading) {
        entry.definition.emplace_back();
        entry.definition.back().reading.insert(reading.key);
    };
    for (const auto& reading : readings) {
        if (reading.restricts.empty()) {
            for (Entry& entry : entries) {
                insertReading(entry, reading);
            }
        } else {
            for (const std::string& restrict : reading.restricts) {
                const auto entryIt = ranges::find_if(entries, [&restrict](const Entry& entry) {
                    return entry.key.front() == restrict;
                });
                if (entryIt == entries.end()) {
                    spdlog::critical("restrict {} not found", restrict);
                }
                insertReading(*entryIt, reading);
            }
        }
    }
    assert(!entries.empty());
}

void spreadSenses(std::vector<Entry>& entries, const std::vector<Sense>& senses)
{
    // auto
    auto insertSenseToDefinition = [](Definition& def, const Sense& sense) {
        def.glossary.insert(sense.meanings.begin(), sense.meanings.end());
        ranges::transform(sense.partOfSpeech, std::inserter(def.pos, def.pos.begin()), parsePOS);
    };
    auto insertSense = [&](Entry& entry, const Sense& sense) {
        // entry.definition.emplace_back();
        // entry.definition.back().reading.push_back(reading.key);
        for (Definition& def : entry.definition) {
            insertSenseToDefinition(def, sense);
        }
    };
    auto insertSenseRestrictedReading = [&](Entry& entry, const Sense& sense) {
        for (Definition& def : entry.definition) {
            const auto& readingIt = ranges::find_if(sense.restrictReading, [&def](const std::string& restrictReading) {
                return restrictReading == *def.reading.begin();
            });
            if (readingIt != sense.restrictReading.end()) {
                insertSenseToDefinition(def, sense);
            }
        };
    };
    for (const Sense& sense : senses) {
        if (sense.restrictKanji.empty() && sense.restrictReading.empty()) {
            for (Entry& entry : entries) {
                insertSense(entry, sense);
                // spdlog::info("insert sense");
            }
        } else if (sense.restrictKanji.empty()) {
            for (Entry& entry : entries) {
                insertSenseRestrictedReading(entry, sense);
            }
        } else {
            for (Entry& entry : entries) {
                const auto& kanjiIt = ranges::find_if(sense.restrictKanji, [&entry](const std::string& restrictKanji) {
                    return restrictKanji == entry.key.front();
                });
                if (kanjiIt != sense.restrictKanji.end()) {
                    if (sense.restrictReading.empty()) {
                        insertSense(entry, sense);
                    } else {
                        insertSenseRestrictedReading(entry, sense);
                    }
                }
            }
        }
    }
}

void joinDefinitions(std::vector<Definition>& definitions)
{
    static int joined = 0;
    const int tempJoined = joined;
    for (auto definition = definitions.begin(); definition < definitions.end();) {
        if (const auto found = std::find_if(std::next(definition), definitions.end(),
                                            [&definition](const Definition& forwardDefinition) {
                                                return forwardDefinition.pos == definition->pos
                                                       && forwardDefinition.glossary == definition->glossary;
                                            });
            found != definitions.end()) {
            found->reading.insert(definition->reading.begin(), definition->reading.end());
            definition = definitions.erase(definition);
            joined++;
            // spdlog::info("erase");
        } else {
            definition++;
        }
    }
    if (joined != tempJoined) {
        // spdlog::info("joinedDefinitions: {}", joined);
    }
}

void joinEntries(std::vector<Entry>& entries)
{
    static int joined = 0;
    const int tempJoined = joined;
    for (auto entry = entries.begin(); entry < entries.end();) {
        if (const auto found = std::find_if(std::next(entry), entries.end(), [&entry](const Entry& forwardEntry) {
                return entry->definition == forwardEntry.definition;
            });
            found != entries.end()) {
            found->key.insert(found->key.end(), entry->key.begin(), entry->key.end());
            assert(found != entry);
            entry = entries.erase(entry);
            joined++;
            // assert(!entries.empty());
            // spdlog::info("joined");
        } else {
            entry++;
        }
    }
    if (joined != tempJoined) {
        // spdlog::info("joinedEntries: {}", joined);
    }
    // assert(!entries.empty());
}

auto parseEntry(const pugi::xml_node& node) -> std::vector<Entry>
{
    std::vector<KanjiElement> kanjis;
    std::vector<ReadingElement> readings;
    std::vector<Sense> senses;

    std::vector<Entry> entries;

    for (const auto& element : node.children()) {
        // spdlog::info("name: {}: {} ", a.name(), a.value());
        if (element.name() == KanjiElement::s_base) {
            kanjis.push_back(parseKanjiElement(element));
        }
        if (element.name() == ReadingElement::s_base) {
            readings.push_back(parseReadingElement(element));
        }
        if (element.name() == Sense::s_base) {
            senses.push_back(parseSense(element));
        }
    }

    if (!kanjis.empty()) {
        ranges::transform(kanjis, std::back_inserter(entries), [](const KanjiElement& kanji) -> Entry {
            Entry entry;
            entry.key.push_back(kanji.key);
            return entry;
        });
    } else {
        ranges::transform(readings, std::back_inserter(entries), [](const ReadingElement& reading) -> Entry {
            Entry entry;
            entry.key.push_back(reading.key);
            return entry;
        });
    }

    spreadReadings(entries, readings);
    spreadSenses(entries, senses);
    // need to join definitions also after
    for (Entry& entry : entries) {
        joinDefinitions(entry.definition);
    }
    joinEntries(entries);

    // static std::size_t entryCount = 0;
    // entryCount += entries.size();
    // for (const Entry& entry : entries) {
    //     spdlog::info("Key: ", fmt::join(entry.definition.front().reading, ", "));
    //     // spdlog::info("Key: ", entry.key.front());
    // }
    // if (entries.size() > 1) {
    //     spdlog::info("----------------");
    //     for (const Entry& entry : entries) {
    //         spdlog::info("Keys: {}", fmt::join(entry.key, ", "));
    //         for (const Definition& definition : entry.definition) {
    //             spdlog::info("   reading: {}", fmt::join(definition.reading, ", "));
    //             spdlog::info("   gloss: {}", fmt::join(definition.glossary, ", "));
    //         }
    //     }
    //     // spdlog::info("EntryCount: {}", entryCount);
    // }

    // if (entries.size() > 1) {
    //     spdlog::info("multiple entries");
    // } else if (entries.front().key.size() > 1) {
    //     spdlog::info("multiple kanji");
    // }

    // if (senses.size() > 1) {
    //     for (const auto& elem : senses) {
    //         fmt::print("{} -:- ", fmt::join(elem.meanings,", "));
    //     }
    //     fmt::print("\n");
    // }
    return entries;
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
        std::vector<Entry> singleNodeEntries = parseEntry(entry);
        entries.insert(entries.end(), singleNodeEntries.begin(), singleNodeEntries.end());

        // }
        count++;
        // if (count > 10) {
        //     break;
        // }
        // count++;
    }
    for (const auto& [index, entry] : views::enumerate(entries)) {
        for (const std::string& key : entry.key) {
            keyToIndex[key].push_back(index);
        }
    }
    spdlog::info("found {} entries, got {} singleNodeEntries, map keys: {}", count, entries.size(), keyToIndex.size());
}
} // namespace japaneseDic
