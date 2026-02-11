#pragma once
#include "Word.h"
#include "WordDB.h"
#include "Word_jpn.h"

#include <dictionary/Dictionary.h>
#include <dictionary/DictionaryJpn.h>
#include <dictionary/Key_jpn.h>
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

    void save() override;

    auto lookup(const dictionary::Key_jpn& key) -> std::shared_ptr<Word>;
    auto lookupId(VocableId vocableId) -> std::shared_ptr<Word_jpn>;

    [[nodiscard]] auto wordIsKnown(const std::string& key) -> bool;
    // [[nodiscard]] auto getDictionary() const -> std::shared_ptr<dictionary::Dictionary>;
    // auto extractCharacters() -> std::set<utl::CharU8>;

private:
    auto lookupId_baseWord(VocableId vocableId) -> std::shared_ptr<Word> override;
    void loadOld();
    void load();
    void parse(const std::string& str);
    static constexpr auto progressDbFilenameOld = "progressVocablesJpnold.zdb";
    static constexpr auto progressDbFilename = "progressVocablesJpn.zdb";
    std::shared_ptr<zikhron::Config> config;
    std::shared_ptr<dictionary::DictionaryJpn> dictionaryJpn;
    using WordPtr = database::WordPtr_jpn;
    std::vector<std::shared_ptr<Word_jpn>> words;
    std::map<std::string, WordPtr> key_word;

    using DicKey = dictionary::DicKey_jpn;
    std::map<DicKey, WordPtr> dicKey_word;
};

} // namespace database
