#include "WordDB.h"

#include "VocableProgress.h" // IWYU pragma: keep (isNewVocable)
#include "Word.h"

#include <dictionary/ZH_Dictionary.h>
#include <misc/Config.h>
#include <misc/Identifier.h>
#include <utils/format.h>
#include <utils/string_split.h>

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iterator>
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

WordDB::WordDB(std::shared_ptr<zikhron::Config> _config)
    : config{std::move(_config)}
    , dictionary{std::make_shared<ZH_Dictionary>(config->Dictionary())}

{
    load();
}

auto WordDB::lookup(const std::string& key) -> std::shared_ptr<Word>
{
    if (key_word.contains(key)) {
        return key_word.at(key);
    }
    auto entryVectorFromKey = dictionary->entryVectorFromKey(key);
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

auto WordDB::getDictionary() const -> std::shared_ptr<const ZH_Dictionary>
{
    return dictionary;
}

void WordDB::load()
{
    auto fileContent = utl::load_string_file(config->DatabaseDirectory() / s_fn_progressVocableDB);
    auto numberOfVocables = ranges::count(fileContent, '\n');
    words.reserve(static_cast<size_t>(numberOfVocables));
    parse(fileContent);
    spdlog::info("numberOfVocables: {}", numberOfVocables);
    ranges::transform(words, std::inserter(key_word, key_word.begin()),
                      [](const std::shared_ptr<Word>& word) -> std::pair<std::string, std::shared_ptr<Word>> {
                          return {word->Key(), word};
                      });
}

void WordDB::save()
{
    auto out = std::ofstream{config->DatabaseDirectory() / s_fn_progressVocableDB};
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

} // namespace database