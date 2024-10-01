#pragma once
#include <misc/Language.h>
#include "Word.h"

#include <dictionary/DictionaryChi.h>
#include <misc/Config.h>
#include <misc/Identifier.h>

#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace database {
using namespace std::literals;

class WordDB
{
    static constexpr auto s_fn_progressVocableDB = "progressVocables.zdb"sv;

public:
    WordDB(std::shared_ptr<zikhron::Config> config, Language language);
    virtual ~WordDB() = default;

    WordDB(const WordDB&) = delete;
    WordDB(WordDB&&) = delete;
    auto operator=(const WordDB&) -> WordDB& = delete;
    auto operator=(WordDB&&) -> WordDB& = delete;

    void save();

    auto lookup(const std::string& key) -> std::shared_ptr<Word>;
    auto lookupId(VocableId vocableId) -> std::shared_ptr<Word>;

    [[nodiscard]] auto wordIsKnown(const std::string& key) const -> bool;
    [[nodiscard]] auto getDictionary() const -> std::shared_ptr<const dictionary::DictionaryChi>;

private:
    void load();
    void parse(const std::string& str);
    void depcrLoadFromJson();
    std::shared_ptr<zikhron::Config> config;
    std::shared_ptr<dictionary::DictionaryChi> dictionary;
    std::vector<std::shared_ptr<Word>> words;
    std::map<std::string, std::shared_ptr<Word>> key_word;
};

} // namespace database
