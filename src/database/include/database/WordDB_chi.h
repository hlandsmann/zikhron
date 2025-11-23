#pragma once
#include "Word.h"
#include "WordDB.h"
#include "Word_chi.h"

#include <dictionary/DictionaryChi.h>
#include <misc/Config.h>
#include <misc/Identifier.h>
#include <misc/Language.h>
#include <utils/StringU8.h>

#include <filesystem>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace database {
using namespace std::literals;

class WordDB_chi : public WordDB
{
public:
    WordDB_chi(std::shared_ptr<zikhron::Config> config,
               std::shared_ptr<dictionary::DictionaryChi> dictionary,
               Language language);

    void save() override;

    auto lookup(const std::string& key) -> std::shared_ptr<Word_chi>;
    auto lookupId(VocableId vocableId) -> std::shared_ptr<Word_chi>;

    [[nodiscard]] auto wordIsKnown(const std::string& key) -> bool;
    // [[nodiscard]] auto getDictionary() const -> std::shared_ptr<dictionary::Dictionary>;
    auto extractCharacters() -> std::set<utl::CharU8>;

private:
    auto lookupId_baseWord(VocableId vocableId) -> std::shared_ptr<Word> override;
    void load();
    void parse(const std::string& str);
    static constexpr auto progressDbFilename = "progressVocablesChi.zdb";
    std::shared_ptr<zikhron::Config> config;
    std::shared_ptr<dictionary::DictionaryChi> dictionaryChi;
    std::vector<std::shared_ptr<Word_chi>> words;
    std::map<std::string, std::shared_ptr<Word_chi>> key_word;
};

} // namespace database
