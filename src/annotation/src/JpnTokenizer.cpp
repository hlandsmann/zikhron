#include "JpnTokenizer.h"

#include "detail/JumanppWrapper.h"

#include <dictionary/JpnDictionary.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace annotation {
JpnTokenizer::JpnTokenizer(std::shared_ptr<dictionary::JpnDictionary> _jpnDictionary)
    : jumanppWrapper{std::make_shared<JumanppWrapper>()}
    , jpnDictionary{std::move(_jpnDictionary)}
{}

auto JpnTokenizer::tokenize(const std::string& text) const -> std::vector<JpnToken>
{
    auto jumanppTokens = jumanppWrapper->tokenize(text);
    spdlog::info("{}", text);

    for (const auto& jumanppToken : jumanppTokens) {
        spdlog::info("{}, - {}, --- {}", jumanppToken.surface, jumanppToken.baseform, jumanppToken.canonicForm);

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
            spdlog::info("    rf: {}", *entry.definition.front().glossary.begin());
    }

    return {};
}
} // namespace annotation
