#include "WordDB.h"

#include "Word.h"

#include <dictionary/ZH_Dictionary.h>
#include <misc/Config.h>
#include <misc/Identifier.h>
#include <spaced_repetition/VocableProgress.h>
#include <spdlog/spdlog.h>
#include <utils/string_split.h>

#include <algorithm>
#include <cstddef>
#include <exception>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iterator>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>
namespace ranges = std::ranges;
namespace views = std::views;
namespace fs = std::filesystem;

namespace {
auto load_string_file(const fs::path& filename) -> std::string
{
    std::string result;
    std::ifstream file;
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    file.open(filename, std::ios_base::binary);
    auto sz = static_cast<std::size_t>(file_size(filename));
    result.resize(sz, '\0');
    file.read(result.data(), static_cast<std::streamsize>(sz));
    return result;
}

using vocId_vocId_map = std::map<VocableId, VocableId>;

} // namespace

namespace annotation {

WordDB::WordDB(std::shared_ptr<zikhron::Config> _config)
    : config{std::move(_config)}
    , dictionary{std::make_shared<ZH_Dictionary>(config->Dictionary())}

{
    load();
}

WordDB::~WordDB()
{
    // save();
    spdlog::info("Unknown: {}", fmt::join(unknown, ", "));
    spdlog::info("Known: {}", fmt::join(known, ", "));
    spdlog::info("unknown: {}, known: {}", unknown.size(), known.size());
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
    auto word = std::make_shared<Word>(std::move(entryVectorFromKey));
    words.push_back(word);
    key_word.insert({key, word});
    return word;
}

void WordDB::load()
{
    auto fileContent = load_string_file(config->DatabaseDirectory() / s_fn_progressVocableDB);
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
}

void WordDB::parse(const std::string& str)
{
    auto iss = std::istringstream{str};
    for (std::string line; std::getline(iss, line);) {
        words.push_back(std::make_shared<Word>(line, dictionary));
    }
}

} // namespace annotation
