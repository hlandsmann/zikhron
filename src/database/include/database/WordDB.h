#pragma once
#include "Word.h"

#include <dictionary/Dictionary.h>
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

class WordDB
{
public:
    WordDB(std::shared_ptr<zikhron::Config> config,
           Language language);
    virtual ~WordDB() = default;

    WordDB(const WordDB&) = delete;
    WordDB(WordDB&&) = delete;
    auto operator=(const WordDB&) -> WordDB& = delete;
    auto operator=(WordDB&&) -> WordDB& = delete;

    virtual void save() = 0;

    // auto lookup(const std::string& key) -> std::shared_ptr<Word>;
    virtual auto lookupId_baseWord(VocableId vocableId) -> std::shared_ptr<Word> = 0;
    //
    // [[nodiscard]] auto wordIsKnown(const std::string& key) const -> bool;
    // [[nodiscard]] auto getDictionary() const -> std::shared_ptr<dictionary::Dictionary>;
    auto extractCharacters() -> std::set<utl::CharU8>;

protected:
    // [[nodiscard]] auto getWords() -> std::vector<std::shared_ptr<Word>>& { return words; };

    // [[nodiscard]] auto getKeyWords() -> std::map<std::string, std::shared_ptr<Word>>& { return key_word; };

private:
    std::map<Language, std::filesystem::path> languageToProgressDbFileNames =
            {{Language::chinese, "progressVocablesChi.zdb"},
             {Language::japanese, "progressVocablesJpn.zdb"}};
    std::filesystem::path progressDbFilename;
    std::shared_ptr<zikhron::Config> config;
    // std::shared_ptr<dictionary::Dictionary> dictionary;
    // std::vector<std::shared_ptr<Word>> words;
};

} // namespace database
