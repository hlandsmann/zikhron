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

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <memory>
#include <regex>
#include <string>
#include <utility>
#include <vector>

namespace annotation {
TokenizerJpn::TokenizerJpn(std::shared_ptr<database::WordDB> _wordDB)
    : mecab{std::make_shared<Mecab>()}
    , wordDB{std::dynamic_pointer_cast<database::WordDB_jpn>(_wordDB)}
    , jpnDictionary{std::dynamic_pointer_cast<dictionary::DictionaryJpn>(wordDB->getDictionary())}
    , log{std::make_unique<spdlog::logger>("", std::make_shared<spdlog::sinks::null_sink_mt>())}
{}

auto interleave(std::regex filter, std::vector<Token> words, std::string text) -> std::vector<Token>
{
    // spdlog::info("`{}`", text);
    // spdlog::info("{}", fmt::join(words, ", "));
    std::sregex_iterator it(text.begin(), text.end(), filter);
    std::sregex_iterator end;
    size_t lastPos = 0;
    std::vector<Token> result;
    auto itWords = words.begin();
    for (; it != end; ++it) {
        const std::smatch& match = *it;
        std::string before = text.substr(lastPos, static_cast<std::size_t>(match.position()) - lastPos);
        std::size_t lw = 0;
        while (lw < before.length() && itWords < words.end()) {
            auto pos = before.find(*itWords, lw);
            if (pos == std::string::npos) {
                result.emplace_back(before.substr(lw));
                break;
            }
            if (pos > lw) {
                // spdlog::info("push, pos: {}, lw: {}, substr: {}", pos, lw, before.substr(lw, pos - lw));
                result.emplace_back(before.substr(lw, pos - lw));
                lw = pos;

                continue;
            }
            lw += itWords->string().length();
            // spdlog::warn("{}|{}: `{}` ({})", before.length(), lw, *itWords, before);
            result.push_back(*itWords);
            itWords++;
        }
        assert(lw <= before.length());
        itWords++;
        result.emplace_back(match.str());

        lastPos = static_cast<std::size_t>(match.position() + match.length());
    }
    if (itWords < words.end()) {
        std::copy(itWords, words.end(), std::back_inserter(result));
    }
    return result;
}

auto TokenizerJpn::split(const std::string& text) const -> std::vector<Token>
{
    // std::regex bracketed("\\(.*?\\)");
    std::regex bracketed("(（.*?）|\\(.*?\\))");
    std::string filterredText = std::regex_replace(text, bracketed, "|");
    // spdlog::info("`{}`", filterredText);
    auto jumanppTokens = mecab->split(filterredText);
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
    result = interleave(bracketed, result, text);
    return result;
}

void TokenizerJpn::setDebugSink(spdlog::sink_ptr sink)
{
    log = std::make_unique<spdlog::logger>("", sink);
    mecab->setDebugSink(sink);
    jpnDictionary->setDebugSink(sink);
}

} // namespace annotation
