#include "AdaptJiebaDict.h"

#include <dictionary/ZH_Dictionary.h>
#include <utils/StringU8.h>
#include <utils/spdlog.h>
#include <utils/string_split.h>

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
namespace ranges = std::ranges;

namespace annotation {

AdaptJiebaDict::AdaptJiebaDict(std::shared_ptr<const ZH_Dictionary> _dictionary)
    : dictionary{std::move(_dictionary)}
    , rules{dictionary}
{
    spdlog::info("Merged dict has {} entries", output.size());
}

void AdaptJiebaDict::load(const std::filesystem::path& dictionaryFileIn)
{
    fileContent = utl::load_string_file(dictionaryFileIn);
    auto numberOfEntries = std::count(fileContent.begin(), fileContent.end(), '\n');
    input.reserve(static_cast<std::size_t>(numberOfEntries));
    auto rest = std::string_view{fileContent};
    while (!rest.empty()) {
        auto key = utl::split_front(rest, ' ');
        auto value = utl::split_front(rest, '\n');
        input.emplace_back(key, value);
    }
    ranges::sort(input,
                 [](const auto& key_value1, const auto& key_value2) {
                     const auto& [key1, value1] = key_value1;
                     const auto& [key2, value2] = key_value2;
                     // if (key1 == key2) {
                     //     return value1 < value2;
                     // }
                     return key1 < key2;
                 }

    );
}

void AdaptJiebaDict::save(const std::filesystem::path& dictionaryFileOut)
{
    auto out = std::ofstream{dictionaryFileOut};
    for (const auto& [key, value] : output) {
        out << fmt::format("{} {}\n", key, value);
    }
}

void AdaptJiebaDict::saveUserDict()
{
    auto out = std::ofstream{user_dict_path};
    for (const auto& entry : userDict) {
        out << fmt::format("{}\n", entry);
    }
}

void AdaptJiebaDict::merge()
{
    auto simplified = dictionary->Simplified();
    auto dic_it = simplified.begin();
    auto inp_it = input.begin();
    std::string lastKey;

    while (dic_it != simplified.end() && inp_it != input.end()) {
        if (dic_it->key < inp_it->key) {
            if (dic_it->key != lastKey && utl::StringU8(dic_it->key).length() > 3) {
                userDict.push_back(dic_it->key);
            }
            dic_it++;
            continue;
        }
        if (inp_it->key < dic_it->key) {
            auto rule = rules.findRule(std::string{inp_it->key});
            if (!rule.empty()) {
                output.push_back(*inp_it);
            }
            inp_it++;
            continue;
        }
        output.push_back(*inp_it);
        lastKey = inp_it->key;
        inp_it++;
    }
}
} // namespace annotation