#pragma once
#include <dictionary/ZH_Dictionary.h>
#include <misc/Config.h>

#include <memory>
#include <string_view>

namespace annotation {

class WordDB
{
    static constexpr std::string_view s_fn_metaVocableSR = "metaVocableSR.json";
    static constexpr std::string_view s_fn_vocableChoices = "vocableChoices.json";

public:
    WordDB(std::shared_ptr<zikhron::Config> config);

private:
    void depcrLoadFromJson();
    std::shared_ptr<zikhron::Config> config;
    std::shared_ptr<ZH_Dictionary> dictionary;
};

} // namespace annotation
