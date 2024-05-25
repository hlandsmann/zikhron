#pragma once
#include "Word.h"

#include <dictionary/ZH_Dictionary.h>
#include <misc/Config.h>
#include <misc/Identifier.h>

#include <map>
#include <memory>
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace annotation {
using namespace std::literals;

class WordDB
{
    static constexpr auto s_fn_metaVocableSR = "metaVocableSR.json"sv;
    static constexpr auto s_fn_vocableChoices = "vocableChoices.json"sv;
    static constexpr auto s_fn_progressVocableDB = "progressVocables.db"sv;

public:
    WordDB(std::shared_ptr<zikhron::Config> config);
    virtual ~WordDB();

    WordDB(const WordDB&) = delete;
    WordDB(WordDB&&) = delete;
    auto operator=(const WordDB&) -> WordDB& = delete;
    auto operator=(WordDB&&) -> WordDB& = delete;

    void save();

    auto lookup(const std::string& key) -> std::shared_ptr<Word>;
    auto lookupId(VocableId vocableId) -> std::shared_ptr<Word>;

    [[nodiscard]] auto wordIsKnown(const std::string& key) const -> bool;
    [[nodiscard]] auto getDictionary() const -> std::shared_ptr<const ZH_Dictionary>;

private:
    void load();
    void parse(const std::string& str);
    void depcrLoadFromJson();
    std::shared_ptr<zikhron::Config> config;
    std::shared_ptr<ZH_Dictionary> dictionary;
    std::vector<std::shared_ptr<Word>> words;
    std::map<std::string, std::shared_ptr<Word>> key_word;
};

} // namespace annotation
