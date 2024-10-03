#include "TokenizerJpn.h"

#include "Token.h"
#include "detail/JumanppWrapper.h"

#include <dictionary/DictionaryJpn.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace annotation {
TokenizerJpn::TokenizerJpn(std::shared_ptr<database::WordDB> wordDB)
    : jumanppWrapper{std::make_shared<JumanppWrapper>()}
    , jpnDictionary{std::dynamic_pointer_cast<const dictionary::DictionaryJpn>(wordDB->getDictionary())}
{}

auto TokenizerJpn::split(const std::string& text) const -> std::vector<Token>
{
    auto jumanppTokens = jumanppWrapper->tokenize(text);
    std::vector<Token> result;
    spdlog::info("{}", text);

    for (const auto& jumanppToken : jumanppTokens) {
        spdlog::info("{}, - {}, --- {}", jumanppToken.surface, jumanppToken.baseform, jumanppToken.canonicForm);
        result.emplace_back(jumanppToken.surface);

        auto entry = jpnDictionary->getEntryByKanji(jumanppToken.baseform);
        if (!entry.kanji.empty()) {
            spdlog::info("    bf: {}", *entry.definition.front().glossary.begin());
            continue;
        }
        entry = jpnDictionary->getEntryByKanji(jumanppToken.surface);
        if (!entry.kanji.empty()) {
            spdlog::info("    sf: {}", *entry.definition.front().glossary.begin());
        }
        entry = jpnDictionary->getEntryByReading(jumanppToken.baseform);
        if (!entry.definition.empty()) {
            spdlog::info("    rf: {}", *entry.definition.front().glossary.begin());
        }
    }
    spdlog::info("full: {}", fmt::join(result, ","));
    return result;
}
} // namespace annotation
