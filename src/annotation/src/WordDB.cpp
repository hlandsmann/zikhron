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

// @deprecated
template<class key_type, class mapped_value>
auto jsonToMap(const nlohmann::json& jsonMeta) -> std::map<key_type, mapped_value>
{
    static constexpr std::string_view s_content = "content";
    std::map<key_type, mapped_value> map;
    const auto& content = jsonMeta.at(std::string(s_content));
    using sr_t = typename std::decay_t<decltype(map)>::mapped_type;
    ranges::transform(content, std::inserter(map, map.begin()), &sr_t::fromJson);
    return map;
}

// @deprecated
auto loadJsonFromFile(const std::filesystem::path& fn) -> nlohmann::json
{
    std::ifstream ifs(fn);
    return nlohmann::json::parse(ifs);
}

using vocId_vocId_map = std::map<VocableId, VocableId>;

// @deprecated
auto loadVocableChoices(const std::filesystem::path& vocableChoicesPath) -> vocId_vocId_map
{
    vocId_vocId_map vocableChoices;
    try {
        nlohmann::json choicesJson = loadJsonFromFile(vocableChoicesPath);
        ranges::transform(choicesJson,
                          std::inserter(vocableChoices, vocableChoices.begin()),
                          [](const nlohmann::json& choice) -> std::pair<VocableId, VocableId> {
                              nlohmann::json id = choice["id"];
                              nlohmann::json map_id = choice["map_id"];

                              return {id, map_id};
                          });
    } catch (const std::exception& e) {
        spdlog::error("Load of vocable choice file failed, Error: {}", e.what());
    }
    return vocableChoices;
}

// @deprecated
auto loadProgressVocablesJson(
        const std::filesystem::path& progressVocablePath) -> std::map<VocableId, VocableProgress>
{
    static constexpr std::string_view s_fn_metaVocableSR = "metaVocableSR.json";
    std::map<VocableId, VocableProgress> id_progress;
    try {
        nlohmann::json jsonVocable = loadJsonFromFile(progressVocablePath);
        id_progress = jsonToMap<VocableId, VocableProgress>(jsonVocable);
        spdlog::debug("Vocable SR file {} loaded!", s_fn_metaVocableSR);
    } catch (const std::exception& e) {
        spdlog::error("Vocabulary SR load for {} failed! Exception {}", progressVocablePath.c_str(), e.what());
    }
    return id_progress;
}
} // namespace

namespace annotation {

WordDB::WordDB(std::shared_ptr<zikhron::Config> _config)
    : config{std::move(_config)}
    // , dictionary{std::make_shared<ZH_Dictionary>(config->Dictionary())}

{
    // depcrLoadFromJson();
    load();
}

void WordDB::load()
{
    auto fileContent = load_string_file(config->DatabaseDirectory() / s_fn_progressVocableDB);
    auto numberOfVocables = ranges::count(fileContent, '\n');
    words.reserve(static_cast<size_t>(numberOfVocables));
    parse(fileContent);
    spdlog::info("numberOfVocables: {}", numberOfVocables);
}

void WordDB::parse(const std::string& str)
{
    auto iss = std::istringstream{str};
    for (std::string line; std::getline(iss, line);) {
        words.push_back(std::make_shared<Word>(line, dictionary));
    }
}

void WordDB::depcrLoadFromJson()
{
    auto pv = loadProgressVocablesJson(config->DatabaseDirectory() / s_fn_metaVocableSR);
    auto vocableChoices = loadVocableChoices(config->DatabaseDirectory() / s_fn_vocableChoices);
    std::vector<Word> words;
    words.reserve(pv.size());

    for (auto& [initial_vocId, vocableProgress] : pv) {
        auto vocId = vocableChoices.contains(initial_vocId)
                             ? vocableChoices.at(initial_vocId)
                             : initial_vocId;

        words.emplace_back(std::make_shared<VocableProgress>(std::move(vocableProgress)), vocId, dictionary);

        // if (vocableChoices.contains(vocId)) {
        //     auto mappedEntry = dictionary->EntryFromPosition(vocableChoices.at(vocId));
        //     spdlog::info("{}, p: {} - {}, m: {} - {}", entry.key, entry.pronounciation, mappedEntry.pronounciation,
        //                  entry.meanings.front(), mappedEntry.meanings.front());
        // }
    }
    // auto out = std::ofstream{config->DatabaseDirectory() / s_fn_progressVocableDB};
    // for (const auto& word : words) {
    //     // fmt::print("{}", word.serialize());
    //     out << word.serialize();
    // }

    spdlog::info("progressVocable size: {}", pv.size());
    spdlog::info("vocableChoices size: {}", vocableChoices.size());
}
} // namespace annotation
