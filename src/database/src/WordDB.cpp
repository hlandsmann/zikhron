#include "WordDB.h"

#include "VocableProgress.h" // IWYU pragma: keep (isNewVocable)
#include "Word.h"

#include <dictionary/Dictionary.h>
#include <dictionary/DictionaryChi.h>
#include <dictionary/DictionaryJpn.h>
#include <misc/Config.h>
#include <misc/Identifier.h>
#include <misc/Language.h>
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
    , dictionary{createDictionary(language, config)}

{
    spdlog::info("WordDB constructed with {}", magic_enum::enum_name(language));
    load();
}

auto WordDB::lookup(const std::string& key) -> std::shared_ptr<Word>
{
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

auto WordDB::lookupId(VocableId vocableId) -> std::shared_ptr<Word>
{
    return words.at(vocableId);
}

auto WordDB::wordIsKnown(const std::string& key) const -> bool
{
    return key_word.contains(key);
}

auto WordDB::getDictionary() const -> std::shared_ptr<const dictionary::Dictionary>
{
    return dictionary;
}

void WordDB::load()
{
    try {
        auto fileContent = utl::load_string_file(config->DatabaseDirectory() / progressDbFilename);
        auto numberOfVocables = ranges::count(fileContent, '\n');
        words.reserve(static_cast<size_t>(numberOfVocables));
        parse(fileContent);
        spdlog::info("WordDB size: {}", numberOfVocables);
        ranges::transform(words, std::inserter(key_word, key_word.begin()),
                          [](const std::shared_ptr<Word>& word) -> std::pair<std::string, std::shared_ptr<Word>> {
                              return {word->Key(), word};
                          });
    } catch (...) {
        spdlog::error("Failed to load WordDB: {}", progressDbFilename.c_str());
    }
}

void WordDB::save()
{
    auto out = std::ofstream{config->DatabaseDirectory() / progressDbFilename};
    for (const auto& word : words) {
        if ((!word->getProgress()->isNewVocable()) || word->isModified()) {
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

void WordDB::parse(const std::string& str)
{
    auto rest = std::string_view{str};
    while (true) {
        auto wordDescription = utl::split_front(rest, '\n');
        if (wordDescription.empty()) {
            break;
        }
        auto vocableId = static_cast<VocableId>(words.size());
        words.push_back(std::make_shared<Word>(wordDescription, vocableId, dictionary));
    }
}

auto WordDB::createDictionary(Language language,
                              std::shared_ptr<zikhron::Config> config) -> std::shared_ptr<dictionary::Dictionary>
{
    switch (language) {
    case Language::chinese:
        return std::make_shared<dictionary::DictionaryChi>(config);
    case Language::japanese:
        return std::make_shared<dictionary::DictionaryJpn>(config);
    }
    std::unreachable();
}

} // namespace database
