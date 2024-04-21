#pragma once
#include "Word.h"

#include <dictionary/ZH_Dictionary.h>
#include <misc/Config.h>

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace annotation {
using namespace std::literals;

class WordDB
{
    static constexpr std::string_view s_fn_metaVocableSR = "metaVocableSR.json";
    static constexpr std::string_view s_fn_vocableChoices = "vocableChoices.json";
    static constexpr auto s_fn_progressVocableDB = "progressVocables.db"sv;

public:
    WordDB(std::shared_ptr<zikhron::Config> config);

private:
    void load();
    void save();
    void parse(const std::string& str);
    void depcrLoadFromJson();
    std::shared_ptr<zikhron::Config> config;
    std::shared_ptr<ZH_Dictionary> dictionary;
    std::vector<std::shared_ptr<Word>> words;
};

} // namespace annotation
