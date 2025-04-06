#pragma once
#include "WordDB.h"
#include "Word_jpn.h"

#include <dictionary/Dictionary.h>
#include <dictionary/DictionaryJpn.h>
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

class WordDB_jpn : public WordDB
{
public:
    WordDB_jpn(std::shared_ptr<zikhron::Config> config,
               std::shared_ptr<dictionary::DictionaryJpn> _dictionary,
               Language language);

    // void save();

    auto lookup(const std::string& key) -> std::shared_ptr<Word>;
    auto lookupId(VocableId vocableId) -> std::shared_ptr<Word>;

    [[nodiscard]] auto wordIsKnown(const std::string& key) -> bool;
    // [[nodiscard]] auto getDictionary() const -> std::shared_ptr<dictionary::Dictionary>;
    // auto extractCharacters() -> std::set<utl::CharU8>;

private:
    // void load();
    // void parse(const std::string& str);
    // static auto createDictionary(Language language,
    //                              std::shared_ptr<zikhron::Config> config) -> std::shared_ptr<dictionary::Dictionary>;
    // std::map<Language, std::filesystem::path> languageToProgressDbFileNames =
    //         {{Language::chinese, "progressVocablesChi.zdb"},
    //          {Language::japanese, "progressVocablesJpn.zdb"}};
    // std::filesystem::path progressDbFilename;
    // std::shared_ptr<zikhron::Config> config;
    std::shared_ptr<dictionary::DictionaryJpn> dictionaryJpn;
    // std::vector<std::shared_ptr<Word>> words;
    // std::map<std::string, std::shared_ptr<Word>> key_word;
};

} // namespace database
