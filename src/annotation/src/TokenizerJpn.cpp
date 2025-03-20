#include "TokenizerJpn.h"

#include "Mecab.h"
#include "Token.h"
#include "detail/JumanppWrapper.h"

#include <database/Word.h>
#include <database/WordDB.h>
#include <dictionary/DictionaryJpn.h>
#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/null_sink.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace annotation {
TokenizerJpn::TokenizerJpn(std::shared_ptr<database::WordDB> _wordDB)
    : mecab{std::make_shared<Mecab>()}
    , wordDB{std::move(_wordDB)}
    , jpnDictionary{std::dynamic_pointer_cast<dictionary::DictionaryJpn>(wordDB->getDictionary())}
    , log{std::make_unique<spdlog::logger>("", std::make_shared<spdlog::sinks::null_sink_mt>())}
{}

auto TokenizerJpn::split(const std::string& text) const -> std::vector<Token>
{
    auto jumanppTokens = mecab->split(text);
    std::vector<Token> result;
    // spdlog::info("{}", text);

    for (const auto& jumanppToken : jumanppTokens) {
        log->info("{},{},{},{},{}", jumanppToken.lemmaType, jumanppToken.pos1, jumanppToken.pos2, jumanppToken.pos3, jumanppToken.pos4);
        // spdlog::info("{}, - {}, --- {}", jumanppToken.surface, jumanppToken.baseform, jumanppToken.canonicForm, jumanppToken.reading);
        auto word = wordDB->lookup(jumanppToken.lemma);
        if (!word) {
            word = wordDB->lookup(jumanppToken.surface);
        }
        if (word) {
            result.emplace_back(jumanppToken.surface, word);
        } else {
            result.emplace_back(jumanppToken.surface);
        }

        // auto entry = jpnDictionary->getEntryByKanji(jumanppToken.baseform);
        // if (!entry.kanji.empty()) {
        //     spdlog::info("    bf: {}", *entry.definition.front().glossary.begin());
        //     // continue;
        // }
        // entry = jpnDictionary->getEntryByKanji(jumanppToken.surface);
        // if (!entry.kanji.empty()) {
        //     spdlog::info("    sf: {}", *entry.definition.front().glossary.begin());
        // }
        // entry = jpnDictionary->getEntryByReading(jumanppToken.baseform);
        // if (!entry.definition.empty()) {
        //     spdlog::info("    rf: {}", *entry.definition.front().glossary.begin());
        // }
    }
    // spdlog::info("full: {}", fmt::join(result, ","));
    return result;
}

void TokenizerJpn::setDebugSink(spdlog::sink_ptr sink)
{
    log = std::make_unique<spdlog::logger>("", sink);
    mecab->setDebugSink(sink);
    jpnDictionary->setDebugSink(sink);
}

} // namespace annotation
