#include "WordDB.h"

#include "Word.h"
#include "ZH_Dictionary.h"

#include <misc/Config.h>
#include <misc/Identifier.h>
#include <spaced_repetition/VocableProgress.h>
#include <utils/format.h>
#include <utils/string_split.h>

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
namespace ranges = std::ranges;
namespace views = std::views;
namespace fs = std::filesystem;

namespace {

using vocId_vocId_map = std::map<VocableId, VocableId>;

} // namespace

namespace annotation {

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
        out << word->serialize();
    }
    spdlog::info("Saved WordDB");
}

void WordDB::parse(const std::string& str)
{
    // auto iss = std::istringstream{str};
    // for (std::string line; std::getline(iss, line);) {
    //     auto vocableId = static_cast<VocableId>(words.size());
    //     words.push_back(std::make_shared<Word>(line, vocableId, dictionary));
    // }
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

} // namespace annotation
