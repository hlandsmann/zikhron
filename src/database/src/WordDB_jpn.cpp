#include "WordDB_jpn.h"

#include "Word.h"
#include "WordDB.h"
#include "Word_jpn.h"

#include <database/SpacedRepetitionData.h>
#include <dictionary/Dictionary.h>
#include <dictionary/DictionaryChi.h>
#include <dictionary/DictionaryJpn.h>
#include <misc/Config.h>
#include <misc/Identifier.h>
#include <misc/Language.h>
#include <utils/StringU8.h>
#include <utils/format.h>
#include <utils/string_split.h>

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <magic_enum/magic_enum.hpp>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
namespace ranges = std::ranges;
namespace fs = std::filesystem;

namespace {

using vocId_vocId_map = std::map<VocableId, VocableId>;

} // namespace

namespace database {

WordDB_jpn::WordDB_jpn(std::shared_ptr<zikhron::Config> _config,
                       std::shared_ptr<dictionary::DictionaryJpn> _dictionary,
                       Language language)
    : WordDB{_config, language}
    , config{std::move(_config)}
    , dictionaryJpn{std::move(_dictionary)}
// , dictionary{createDictionary(language, config)}

{
    spdlog::info("WordDB_jpn constructed with {}", magic_enum::enum_name(language));
    load();
    // const auto& characters = extractCharacters();
    // spdlog::info("Character size: {}", characters.size());
}

void WordDB_jpn::load()
{
    try {
        auto fileContent = utl::load_string_file(config->DatabaseDirectory() / progressDbFilename);
        auto numberOfVocables = ranges::count(fileContent, '\n');
        words.reserve(static_cast<size_t>(numberOfVocables));
        parse(fileContent);
        spdlog::info("WordDB size: {}", numberOfVocables);
        ranges::transform(words, std::inserter(key_word, key_word.begin()),
                          [](const std::shared_ptr<Word_jpn>& word) -> std::pair<std::string, std::shared_ptr<Word_jpn>> {
                              return {word->Key(), word};
                          });
    } catch (...) {
        spdlog::error("Failed to load WordDB: {}", progressDbFilename);
    }
}

auto WordDB_jpn::lookup(const dictionary::Key_jpn& key) -> std::shared_ptr<Word>
{
    if (key_word.contains(key.key)) {
        return key_word.at(key.key);
    }
    auto entryVectorFromKey = dictionaryJpn->entriesFromKey(key.key);
    if (entryVectorFromKey.empty()) {
        return nullptr;
    }
    auto word = std::make_shared<Word_jpn>(std::move(entryVectorFromKey), static_cast<VocableId>(words.size()));
    words.push_back(word);
    key_word.insert({key.key, word});
    return word;
}

auto WordDB_jpn::lookupId(VocableId vocableId) -> std::shared_ptr<Word_jpn>
{
    return words.at(vocableId);
}

auto WordDB_jpn::lookupId_baseWord(VocableId vocableId) -> std::shared_ptr<Word>
{
    return words.at(vocableId);
}

auto WordDB_jpn::wordIsKnown(const std::string& key) -> bool
{
    return key_word.contains(key);
}

void WordDB_jpn::parse(const std::string& str)
{
    auto rest = std::string_view{str};
    while (true) {
        auto wordDescription = utl::split_front(rest, '\n');
        if (wordDescription.empty()) {
            break;
        }
        auto vocableId = static_cast<VocableId>(words.size());
        words.push_back(std::make_shared<Word_jpn>(wordDescription, vocableId, dictionaryJpn));
    }
}

// auto WordDB_jpn::getDictionary() const -> std::shared_ptr<dictionary::Dictionary>
// {
//     return dictionary;
// }

// auto WordDB_jpn::extractCharacters() -> std::set<utl::CharU8>
// {
//     std::set<utl::CharU8> characters;
//     for (const auto& [key, _] : key_word) {
//         auto keyU8 = utl::StringU8{key};
//         for (const utl::CharU8& character : keyU8.getChars()) {
//             characters.insert(character);
//         }
//     }
//     return characters;
// }

// void WordDB_jpn::load()
// {
//     try {
//         auto fileContent = utl::load_string_file(config->DatabaseDirectory() / progressDbFilename);
//         auto numberOfVocables = ranges::count(fileContent, '\n');
//         words.reserve(static_cast<size_t>(numberOfVocables));
//         parse(fileContent);
//         spdlog::info("WordDB_jpn size: {}", numberOfVocables);
//         ranges::transform(words, std::inserter(key_word, key_word.begin()),
//                           [](const std::shared_ptr<Word>& word) -> std::pair<std::string, std::shared_ptr<Word>> {
//                               return {word->Key(), word};
//                           });
//     } catch (...) {
//         spdlog::error("Failed to load WordDB_jpn: {}", progressDbFilename.c_str());
//     }
// }

void WordDB_jpn::save()
{
    spdlog::warn("Save ommitted for WordDB");
    return;

    auto out = std::ofstream{config->DatabaseDirectory() / progressDbFilename};
    for (const auto& word : words) {
        if ((word->getSpacedRepetitionData()->state != database::StudyState::newWord)
            || word->isModified()
            || word->getSpacedRepetitionData()->enabled) {
            out << word->serialize();
        } else {
            // spdlog::info("Removed word: {} - {} -  {}",
            //              word->Key(),
            //              word->getDefinitions().front().pronounciation,
            //              word->getDefinitions().front().meanings.front());
        }
    }
    spdlog::info("Saved WordDB");
}

//
// void WordDB_jpn::parse(const std::string& str)
// {
//     auto rest = std::string_view{str};
//     while (true) {
//         auto wordDescription = utl::split_front(rest, '\n');
//         if (wordDescription.empty()) {
//             break;
//         }
//         auto vocableId = static_cast<VocableId>(words.size());
//         words.push_back(std::make_shared<Word>(wordDescription, vocableId, dictionary));
//     }
// }

// auto WordDB_jpn::createDictionary(Language language,
//                                   std::shared_ptr<zikhron::Config> config) -> std::shared_ptr<dictionary::Dictionary>
// {
//     switch (language) {
//     case Language::chinese:
//         return std::make_shared<dictionary::DictionaryChi>(config);
//     case Language::japanese:
//         return std::make_shared<dictionary::DictionaryJpn>(config);
//     }
//     std::unreachable();
// }

} // namespace database
