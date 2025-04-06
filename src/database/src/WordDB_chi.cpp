#include "WordDB_chi.h"

#include "VocableProgress.h" // IWYU pragma: keep (isNewVocable)
#include "WordDB.h"
#include "Word_chi.h"

#include <database/SpacedRepetitionData.h>
#include <dictionary/Dictionary.h>
#include <dictionary/DictionaryChi.h>
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
#include <magic_enum.hpp>
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

WordDB_chi::WordDB_chi(std::shared_ptr<zikhron::Config> _config,
                       std::shared_ptr<dictionary::DictionaryChi> _dictionary,
                       Language language)
    : WordDB{_config, _dictionary, language}
    // , progressDbFilename{languageToProgressDbFileNames.at(language)}
    // , config{std::move(_config)}

{
    spdlog::info("WordDB_chi constructed with {}", magic_enum::enum_name(language));
    // load();
    // const auto& characters = extractCharacters();
    // spdlog::info("Character size: {}", characters.size());
}

auto WordDB_chi::lookup(const std::string& key) -> std::shared_ptr<Word>
{
    const auto& dictionary = getDictionary();
    auto& words = getWords();
    auto& key_word = getKeyWords();
    if (key_word.contains(key)) {
        return key_word.at(key);
    }
    auto entryVectorFromKey = dictionary->entriesFromKey(key);
    if (entryVectorFromKey.empty()) {
        return nullptr;
    }
    auto word = std::make_shared<Word>(std::move(entryVectorFromKey), static_cast<VocableId>(words.size()));
    words.push_back(word);
    key_word.insert({key, word});
    return word;
}

auto WordDB_chi::lookupId(VocableId vocableId) -> std::shared_ptr<Word>
{
    const auto& words = getWords();
    return words.at(vocableId);
}

auto WordDB_chi::wordIsKnown(const std::string& key)  -> bool
{
    const auto& key_word = getKeyWords();
    return key_word.contains(key);
}

// auto WordDB_chi::getDictionary() const -> std::shared_ptr<dictionary::Dictionary>
// {
//     return dictionary;
// }

// auto WordDB_chi::extractCharacters() -> std::set<utl::CharU8>
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

// void WordDB_chi::load()
// {
//     try {
//         auto fileContent = utl::load_string_file(config->DatabaseDirectory() / progressDbFilename);
//         auto numberOfVocables = ranges::count(fileContent, '\n');
//         words.reserve(static_cast<size_t>(numberOfVocables));
//         parse(fileContent);
//         spdlog::info("WordDB_chi size: {}", numberOfVocables);
//         ranges::transform(words, std::inserter(key_word, key_word.begin()),
//                           [](const std::shared_ptr<Word>& word) -> std::pair<std::string, std::shared_ptr<Word>> {
//                               return {word->Key(), word};
//                           });
//     } catch (...) {
//         spdlog::error("Failed to load WordDB_chi: {}", progressDbFilename.c_str());
//     }
// }

// void WordDB_chi::save()
// {
//     auto out = std::ofstream{config->DatabaseDirectory() / progressDbFilename};
//     for (const auto& word : words) {
//         if ((word->getSpacedRepetitionData()->state != database::StudyState::newWord)
//             || word->isModified()
//             || word->getSpacedRepetitionData()->enabled) {
//             out << word->serialize();
//         } else {
//             // spdlog::info("Removed word: {} - {} -  {}",
//             //              word->Key(),
//             //              word->getDefinitions().front().pronounciation,
//             //              word->getDefinitions().front().meanings.front());
//         }
//     }
//     spdlog::info("Saved WordDB_chi");
// }

// void WordDB_chi::parse(const std::string& str)
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

// auto WordDB_chi::createDictionary(Language language,
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
