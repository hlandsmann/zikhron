#include "WordDB.h"

#include "Word.h"

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

WordDB::WordDB(std::shared_ptr<zikhron::Config> _config,
               Language language)
    : progressDbFilename{languageToProgressDbFileNames.at(language)}
    , config{std::move(_config)}

{
    spdlog::info("WordDB constructed with {}", magic_enum::enum_name(language));
    // load();
    // const auto& characters = extractCharacters();
    // spdlog::info("Character size: {}", characters.size());
}

// auto WordDB::lookup(const std::string& key) -> std::shared_ptr<Word>
// {
//     if (key_word.contains(key)) {
//         return key_word.at(key);
//     }
//     auto entryVectorFromKey = dictionary->entriesFromKey(key);
//     if (entryVectorFromKey.empty()) {
//         return nullptr;
//     }
//     auto word = std::make_shared<Word>(std::move(entryVectorFromKey), static_cast<VocableId>(words.size()));
//     words.push_back(word);
//     key_word.insert({key, word});
//     return word;
// }
//

// auto WordDB::wordIsKnown(const std::string& key) const -> bool
// {
//     return key_word.contains(key);
// }

// auto WordDB::getDictionary() const -> std::shared_ptr<dictionary::Dictionary>
// {
//     return dictionary;
// }

auto WordDB::extractCharacters() -> std::set<utl::CharU8>
{
    // std::set<utl::CharU8> characters;
    // for (const auto& [key, _] : key_word) {
    //     auto keyU8 = utl::StringU8{key};
    //     for (const utl::CharU8& character : keyU8.getChars()) {
    //         characters.insert(character);
    //     }
    // }
    // return characters;
    return {};
}

// void WordDB::save()
// {
//     spdlog::warn("Save ommitted for WordDB");
//     return;
//
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
//     spdlog::info("Saved WordDB");
// }

} // namespace database
