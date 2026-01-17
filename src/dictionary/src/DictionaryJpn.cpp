#include "DictionaryJpn.h"

#include "Kana.h"

#include <misc/Config.h>
#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/null_sink.h>
#include <spdlog/spdlog.h>
#include <utils/format.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <filesystem>
#include <generator>
#include <iterator>
#include <limits>
#include <magic_enum/magic_enum.hpp>
#include <map>
#include <memory>
#include <pugixml.hpp>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>
using namespace std::literals;
namespace ranges = std::ranges;
namespace views = std::views;

namespace dictionary {
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

void spreadReadings(std::vector<DictionaryJpn::InternalEntry>& entries, const std::vector<ReadingElement>& readings)
{
    auto insertReading = [](DictionaryJpn::InternalEntry& entry, const ReadingElement& reading) {
        entry.definitions.emplace_back();
        entry.definitions.back().readings.insert(reading.key);
    };
    for (const auto& reading : readings) {
        if (reading.restricts.empty()) {
            for (DictionaryJpn::InternalEntry& entry : entries) {
                insertReading(entry, reading);
            }
        } else {
            for (const std::string& restrict : reading.restricts) {
                const auto entryIt = ranges::find_if(entries, [&restrict](const DictionaryJpn::InternalEntry& entry) {
                    return entry.kanjis.front() == restrict;
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

void spreadSenses(std::vector<DictionaryJpn::InternalEntry>& entries, const std::vector<Sense>& senses)
{
    // auto
    auto insertSenseToDefinition = [](DictionaryJpn::Definition& def, const Sense& sense) {
        def.glossary.insert(sense.meanings.begin(), sense.meanings.end());
        if (!sense.info.empty()) {
            def.infos.insert(sense.info);
        }
        ranges::transform(sense.partOfSpeech, std::inserter(def.pos, def.pos.begin()), parsePOS);
    };
    auto insertSense = [&](DictionaryJpn::InternalEntry& entry, const Sense& sense) {
        // entry.definition.emplace_back();
        // entry.definition.back().reading.push_back(reading.key);
        for (DictionaryJpn::Definition& def : entry.definitions) {
            insertSenseToDefinition(def, sense);
        }
    };
    auto insertSenseRestrictedReading = [&](DictionaryJpn::InternalEntry& entry, const Sense& sense) {
        for (DictionaryJpn::Definition& def : entry.definitions) {
            const auto& readingIt = ranges::find_if(sense.restrictReading, [&def](const std::string& restrictReading) {
                return restrictReading == *def.readings.begin();
            });
            if (readingIt != sense.restrictReading.end()) {
                insertSenseToDefinition(def, sense);
            }
        };
    };
    for (const Sense& sense : senses) {
        if (sense.restrictKanji.empty() && sense.restrictReading.empty()) {
            for (DictionaryJpn::InternalEntry& entry : entries) {
                insertSense(entry, sense);
                // spdlog::info("insert sense");
            }
        } else if (sense.restrictKanji.empty()) {
            for (DictionaryJpn::InternalEntry& entry : entries) {
                insertSenseRestrictedReading(entry, sense);
            }
        } else {
            for (DictionaryJpn::InternalEntry& entry : entries) {
                const auto& kanjiIt = ranges::find_if(sense.restrictKanji, [&entry](const std::string& restrictKanji) {
                    return restrictKanji == entry.kanjis.front();
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

void joinDefinitions(std::vector<DictionaryJpn::Definition>& definitions)
{
    static int joined = 0;
    const int tempJoined = joined;
    for (auto definition = definitions.begin(); definition < definitions.end();) {
        if (const auto found = std::find_if(std::next(definition), definitions.end(),
                                            [&definition](const DictionaryJpn::Definition& forwardDefinition) {
                                                return forwardDefinition.pos == definition->pos
                                                       && forwardDefinition.glossary == definition->glossary;
                                            });
            found != definitions.end()) {
            found->readings.insert(definition->readings.begin(), definition->readings.end());
            found->infos.insert(definition->infos.begin(), definition->infos.end());
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

void joinEntries(std::vector<DictionaryJpn::InternalEntry>& entries)
{
    static int joined = 0;
    const int tempJoined = joined;
    for (auto entry = entries.begin(); entry < entries.end();) {
        if (const auto found = std::find_if(std::next(entry), entries.end(), [&entry](const DictionaryJpn::InternalEntry& forwardEntry) {
                return entry->definitions == forwardEntry.definitions;
            });
            found != entries.end()) {
            found->kanjis.insert(found->kanjis.end(), entry->kanjis.begin(), entry->kanjis.end());
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

auto parseEntry(const pugi::xml_node& node) -> std::vector<DictionaryJpn::InternalEntry>
{
    std::vector<KanjiElement> kanjis;
    std::vector<ReadingElement> readings;
    std::vector<Sense> senses;

    std::vector<DictionaryJpn::InternalEntry> entries;

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
        ranges::transform(kanjis, std::back_inserter(entries), [](const KanjiElement& kanji) -> DictionaryJpn::InternalEntry {
            DictionaryJpn::InternalEntry entry;
            entry.kanjis.push_back(kanji.key);
            return entry;
        });
    } else {
        entries.emplace_back();
    }

    spreadReadings(entries, readings);
    spreadSenses(entries, senses);
    // need to join definitions also after
    for (DictionaryJpn::InternalEntry& entry : entries) {
        joinDefinitions(entry.definitions);
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

DictionaryJpn::DictionaryJpn(std::shared_ptr<zikhron::Config> config)
    : log{std::make_unique<spdlog::logger>("", std::make_shared<spdlog::sinks::null_sink_mt>())}
{
    pugi::xml_document doc;
    const auto& filename = config->Dictionary() / "JMdict_e";
    pugi::xml_parse_result result = doc.load_file(filename.c_str());

    spdlog::info("Loading japanese dictioary: {}", result.description());
    pugi::xml_node jmDict = doc.child("JMdict");

    int count = 0;
    for (const pugi::xml_node& entry : jmDict.children("entry")) {
        // if (count > 1000 && count < 1010) {
        std::vector<InternalEntry> singleNodeEntries = parseEntry(entry);
        entries.insert(entries.end(), singleNodeEntries.begin(), singleNodeEntries.end());

        // }
        count++;
        // if (count > 10) {
        //     break;
        // }
        // count++;
    }
    for (const auto& [indexEntry, entry] : views::enumerate(entries)) {
        for (const std::string& kanji : entry.kanjis) {
            kanjiToIndex[kanji].push_back(static_cast<std::size_t>(indexEntry));
        }
        for (const auto& [indexDefinition, definition] : views::enumerate(entry.definitions)) {
            for (const std::string& reading : definition.readings) {
                readingToIndex[reading].push_back({.indexEntry = static_cast<std::size_t>(indexEntry),
                                                   .indexDefinition = static_cast<std::size_t>(indexDefinition)});
            }
        }
    }
    spdlog::info("found {} entries, got {} singleNodeEntries, map kanji: {}, map reading: {}",
                 count, entries.size(), kanjiToIndex.size(), readingToIndex.size());
}

auto DictionaryJpn::entriesFromKey(const std::string& key) const -> std::vector<EntryJpn>
{
    std::vector<EntryJpn> result;
    auto internalEntryByKanji = getEntryByKanji(key);

    if (!internalEntryByKanji.definitions.empty()) {
        for (const auto& definition : internalEntryByKanji.definitions) {
            std::vector<std::string> glossary;
            ranges::copy(definition.glossary, std::back_inserter(glossary));
            result.push_back({.key = key,
                              .pronounciation = ranges::to<std::vector<std::string>>(definition.readings),
                              .meanings = glossary});
        }

    } else {
        auto internalEntryByReading = getEntryByReading(key);
        for (const auto& definition : internalEntryByReading.definitions) {
            std::vector<std::string> glossary;
            ranges::copy(definition.glossary, std::back_inserter(glossary));
            result.push_back({.key = key,
                              .pronounciation = ranges::to<std::vector<std::string>>(definition.readings),
                              .meanings = glossary});
        }
    }

    return result;
}

auto DictionaryJpn::contains(const std::string& key) const -> bool
{
    return kanjiToIndex.contains(key) || readingToIndex.contains(key);
}

auto DictionaryJpn::getEntryByKey(const Key_jpn& key) const -> DicEntry_jpn
{
    if (key.key.empty()) {
        return {};
    }
    if (Kana::isKana(key.key)) {
        return getEntryByReading(key.key, key.hint);
    }
    return {getEntryByKanji(key.key, key.hint, key.reading)};
}

auto DictionaryJpn::getEntryByReading(const std::string& reading, const std::string& hint) const -> DicEntry_jpn
{
    if (!readingToIndex.contains(reading)) {
        return {};
    }
    DicEntry_jpn entry;

    const std::vector<ReadingPosition>& positions = readingToIndex.at(reading);

    bool hintIsValidKana = false;
    if (Kana::isKana(hint)) {
        hintIsValidKana = ranges::any_of(positions,
                                         [this](const ReadingPosition& position) -> bool {
                                             const auto& [indexEntry, indexDefinition] = position;
                                             return entries.at(indexEntry).kanjis.empty();
                                         });
    }
    bool hintIsValidKanji = false;
    if (!hint.empty() && !Kana::isKana(hint)) {
        hintIsValidKanji = ranges::any_of(positions,
                                          [this, &hint](const ReadingPosition& position) -> bool {
                                              const auto& [indexEntry, indexDefinition] = position;
                                              return ranges::contains(entries.at(indexEntry).kanjis, hint);
                                          });
    }
    if (hintIsValidKanji) {
        entry.key.kanji = hint;
    }
    entry.key.reading = reading;

    InternalEntry dicEntry;
    for (const auto& [indexEntry, indexDefinition] : positions) {
        const auto& currentEntry = entries.at(indexEntry);
        if (hintIsValidKanji && !ranges::contains(currentEntry.kanjis, hint)) {
            continue;
        }
        if (hintIsValidKana && !currentEntry.kanjis.empty()) {
            continue;
        }
        const auto& definition = currentEntry.definitions.at(indexDefinition);
        entry.definitions.push_back(DicDef_jpn{
                .kanjis = hintIsValidKanji
                                  ? std::vector<std::string>{hint}
                                  : std::vector<std::string>(currentEntry.kanjis.begin(), currentEntry.kanjis.end()),
                .readings = {reading},
                .meanings = std::vector<std::string>(definition.glossary.begin(), definition.glossary.end()),
        });
    }

    return entry;
}

auto DictionaryJpn::getEntryByKanji(const std::string& kanji, const std::string& hint, const std::string& reading) const
        -> DicEntry_jpn
{
    DicEntry_jpn entry;
    const std::vector<std::size_t>& indices = [this, &kanji, &hint]() -> const std::vector<std::size_t>& {
        static std::vector<std::size_t> empty = {};
        try {
            return kanjiToIndex.at(kanji);
        } catch (...) {
        }
        if (!hint.empty() && !Kana::isKana(hint)) {
            try {
                return kanjiToIndex.at(hint);
            } catch (...) {
            }
        }
        return empty;
    }();
    if (indices.empty()) {
        return {};
    }

    bool readingIsValid = false;
    if (!reading.empty() && Kana::isKana(reading)) {
        readingIsValid = ranges::any_of(indices, [this, &reading](std::size_t index) -> bool {
            const auto& definitions = entries[index].definitions;
            return ranges::contains(definitions | views::transform(&Definition::readings) | views::join, reading);
        });
    }
    entry.key.kanji = kanji;
    if (!hint.empty() && !Kana::isKana(hint)) {
        entry.key.kanjiNorm = hint;
    }
    entry.key.reading = reading;
    for (const auto index : indices) {
        const auto& dicEntry = entries[index];
        ranges::transform(dicEntry.definitions
                                  | views::filter([readingIsValid, &reading](const Definition& definition) -> bool {
                                        return !readingIsValid || ranges::contains(definition.readings, reading);
                                    }),
                          std::back_inserter(entry.definitions),
                          [&dicEntry](const Definition& definition) -> DicDef_jpn {
                              return {
                                      .kanjis = std::vector<std::string>(dicEntry.kanjis.begin(),
                                                                         dicEntry.kanjis.end()),
                                      .readings = std::vector<std::string>(definition.readings.begin(),
                                                                           definition.readings.end()),
                                      .meanings = std::vector<std::string>(definition.glossary.begin(),
                                                                           definition.glossary.end()),
                              };
                          });
    }

    return entry;
}

auto DictionaryJpn::getEntryByKanji(const std::string& key) const -> InternalEntry
{
    try {
        const std::vector<std::size_t>& indices = kanjiToIndex.at(key);
        InternalEntry entry;
        entry.kanjis.push_back(key);
        for (const auto index : indices) {
            const auto& definition = entries[index].definitions;
            entry.definitions.insert(entry.definitions.end(), definition.begin(), definition.end());
        }
        return entry;

    } catch (...) {
        // spdlog::error("Failed to get dictionary entry for kanji: {}", key);
        return {};
    }
}

auto DictionaryJpn::getEntryByReading(const std::string& key) const -> InternalEntry
{
    try {
        const std::vector<ReadingPosition>& positions = readingToIndex.at(key);
        InternalEntry entry;
        std::size_t indexEntryCount = 0;
        std::size_t indexEntryLast = std::numeric_limits<std::size_t>::max();
        for (const auto& [indexEntry, indexDefinition] : positions) {
            const auto& currentEntry = entries.at(indexEntry);
            if (indexEntry != indexEntryLast) {
                const auto& kanjis = currentEntry.kanjis;
                if (!entries[indexEntry].kanjis.empty()) {
                    entry.kanjis.insert(entry.kanjis.begin(), kanjis.begin(), kanjis.end());
                }
                indexEntryLast = indexEntry;
                indexEntryCount++;
            }
            entry.definitions.push_back(currentEntry.definitions.at(indexDefinition));
        }
        if (indexEntryCount != 1) {
            log->info("indexEntryCount: {}", indexEntryCount);
        }
        return entry;
    } catch (...) {
        // spdlog::error("Failed to get dictionary entry for Reading: {}", key);
        return {};
    }
}

void DictionaryJpn::setDebugSink(spdlog::sink_ptr sink)
{
    log = std::make_unique<spdlog::logger>("", sink);
}
} // namespace dictionary
