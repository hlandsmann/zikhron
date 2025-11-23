#include "TokenizerJpn.h"

#include "Mecab.h"
#include "Sudachi.h"
#include "Token.h"
// #include "detail/JumanppWrapper.h"

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
TokenizerJpn::TokenizerJpn(std::shared_ptr<database::WordDB_jpn> _wordDB_jpn,
                           std::unique_ptr<Sudachi> _sudachi,
                           std::shared_ptr<dictionary::DictionaryJpn> _dictionaryJpn)
    : mecab{std::make_shared<Mecab>()}
    , sudachi{std::move(_sudachi)}
    , wordDB{std::move(_wordDB_jpn)}
    , jpnDictionary{std::move(_dictionaryJpn)}
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
    const std::string filterredText = std::regex_replace(text, bracketed, "|");
    // spdlog::info("`{}`", filterredText);
    // spdlog::info("full: {}", fmt::join(result, ","));
    std::vector<Token> tokens = split_sudachi(filterredText);
    tokens = interleave(bracketed, tokens, text);
    return tokens;
}

auto TokenizerJpn::split_mecab(const std::string& text) const -> std::vector<Token>
{
    auto mecabTokens = mecab->split(text);
    std::vector<Token> tokens;
    // spdlog::info("{}", text);

    for (const auto& mecabToken : mecabTokens) {
        log->info("{},{},{},{},{}", mecabToken.lemmaType, mecabToken.pos1, mecabToken.pos2, mecabToken.pos3, mecabToken.pos4);
        // spdlog::info("{}, - {}, --- {}", jumanppToken.surface, jumanppToken.baseform, jumanppToken.canonicForm, jumanppToken.reading);
        auto word = wordDB->lookup(mecabToken.lemma);
        if (!word) {
            word = wordDB->lookup(mecabToken.surface);
        }
        if (word) {
            tokens.emplace_back(mecabToken.surface, word);
        } else {
            tokens.emplace_back(mecabToken.surface);
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
    return tokens;
}

auto TokenizerJpn::split_sudachi(const std::string& text) const -> std::vector<Token>
{
    auto sudachiTokens = sudachi->split(text);
    sudachiTokens = sudachi->mergeConjugation(sudachiTokens);
    std::vector<Token> tokens;
    // spdlog::info("{}", text);

    for (const auto& mecabToken : sudachiTokens) {
        log->info("{},{},{},{},{}", mecabToken.surface, mecabToken.normalized_form, mecabToken.dictionary_form, mecabToken.pos1, mecabToken.pos2);
        // spdlog::info("{}, - {}, --- {}", jumanppToken.surface, jumanppToken.baseform, jumanppToken.canonicForm, jumanppToken.reading);
        auto word = wordDB->lookup(mecabToken.dictionary_form);
        if (!word) {
            word = wordDB->lookup(mecabToken.surface);
        }
        if (!word) {
            word = wordDB->lookup(mecabToken.normalized_form);
        }
        if (word) {
            tokens.emplace_back(mecabToken.surface_merged, word);
        } else {
            tokens.emplace_back(mecabToken.surface_merged);
        }
    }
    return tokens;
}

void TokenizerJpn::setDebugSink(spdlog::sink_ptr sink)
{
    log = std::make_unique<spdlog::logger>("", sink);
    mecab->setDebugSink(sink);
    jpnDictionary->setDebugSink(sink);
}

} // namespace annotation
