#include "Sudachi.h"

#include "Token.h"

#include <spdlog/spdlog.h>
#include <utils/ProcessPipe.h>
#include <utils/format.h>
#include <utils/string_split.h>

#include <algorithm>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace ranges = std::ranges;

namespace annotation {
Sudachi::Sudachi(std::shared_ptr<utl::ProcessPipe> _processPipe)
    : processPipe{std::move(_processPipe)}
{
}

auto Sudachi::split(const std::string& text) const -> std::vector<SudachiToken>
{
    const std::string fwdString = fmt::format("{}\n", text);
    processPipe->write(fwdString);
    // spdlog::info("\n{}", text);
    // spdlog::info("\n{}", chunk);
    std::vector<SudachiToken> tokens;
    auto cutText = std::string_view{text};
    while (true) {
        const auto& chunk = processPipe->getChunk();
        if (chunk.empty()) {
            break;
        }
        // fmt::print("\n{}", chunk);
        auto chunkView = std::string_view{chunk};
        while (true) {
            auto line = utl::split_front(chunkView, '\n');
            if (line == "EOS") {
                continue;
            }
            if (line.empty()) {
                break;
            }
            const auto& token = parseLine(line);
            if (cutText.starts_with(token.surface)) {
                cutText = cutText.substr(token.surface.length());
                tokens.push_back(token);
            } else {
                spdlog::error("\"{}\" - cutText: \"{}\" - not found: \"{}\"", text, cutText, token.surface);
            }
            // spdlog::info("{} - {}", tokens.back().surface, tokens.back().extra_info);
        }
        if (isCovered(tokens, text)) {
            break;
        }
        // fmt::print("\"{}\"", text);
        // fmt::print("{}", chunk);
    }
    // for (const auto& token : tokens) {
    //     spdlog::info("{} - {}", token.surface, token.extra_info);
    // }
    return tokens;
}

auto Sudachi::mergeConjugation(const std::vector<SudachiToken>& sudachiTokens) -> std::vector<SudachiToken>
{
    std::vector<SudachiToken> finalTokens;
    SudachiToken bufferToken;
    bool bufferEmpty = true;

    for (const SudachiToken& current : sudachiTokens) {
        if (bufferEmpty) {
            bufferToken = current;
            bufferToken.surface_merged = bufferToken.surface;
            bufferEmpty = false;
        } else {
            // Check if we should merge 'current' into 'bufferToken'
            if (shouldMerge(bufferToken, current)) {
                // MERGE
                bufferToken.surface_merged += current.surface;
                // bufferToken.reading += current.reading;
                // Note: We keep the original POS of the head word (e.g., Verb)
                // because the chain acts as a single verb phrase.
            } else {
                // PUSH BUFFER & START NEW
                finalTokens.push_back(bufferToken);
                bufferToken = current;
                bufferToken.surface_merged = current.surface;
            }
        }
    }
    // Push remaining
    if (!bufferEmpty) {
        finalTokens.push_back(bufferToken);
    }
    return finalTokens;
}

auto Sudachi::shouldMerge(const SudachiToken& prev, const SudachiToken& curr) -> bool
{
    // 1. Check if Previous is a Verb, Adjective, or already an Auxiliary (for chaining)
    bool prevIsBase = (prev.pos1 == "動詞" || prev.pos1 == "形容詞" || prev.pos1 == "助動詞");

    if (!prevIsBase) {
        return false;
    }

    // 2. Check if Current is a "Suffix-like" element

    // Case A: Auxiliary Verb (助動詞) - e.g., 'reru', 'ta', 'da'
    if (curr.pos1 == "助動詞") {
        return true;
    }

    // Case B: Specific Particles that function as conjugations or sentence extenders
    if (curr.pos1 == "助詞") {
        // 1. Conjunctive Particle (接続助詞) - e.g., 'te', 'de' (呼ん-で)
        if (curr.pos2 == "接続助詞") {
            return true;
        }

        // 2. Nominalizing/Phrasal Particle (準体助詞) - e.g., 'n' (来る-ん-だ)
        if (curr.pos2 == "準体助詞") {
            return true;
        }

        // 3. Sentence-ending Particle (終助詞) - e.g., 'na', 'yo' (来るんだ-な)
        if (curr.pos2 == "終助詞") {
            return true;
        }
    }

    return false;
}

// [21:59:00 286] [I] 赤道,赤道,赤道,名詞,普通名詞
// [21:59:00 286] [I] に,に,に,助詞,格助詞
// [21:59:00 286] [I] 沿っ,沿う,沿う,動詞,一般
// [21:59:00 286] [I] て,て,て,助詞,接続助詞
// [21:59:00 286] [I] 旅,旅,旅,名詞,普通名詞
// [21:59:00 286] [I] を,を,を,助詞,格助詞
// [21:59:00 286] [I] し,為る,する,動詞,非自立可能
// [21:59:00 286] [I] まし,ます,ます,助動詞,*
// [21:59:00 286] [I] た,た,た,助動詞,*
// [21:59:00 286] [I] 。,。,。,補助記号,句点
auto Sudachi::parseLine(std::string_view line) -> SudachiToken
{
    return {
            .surface_merged = "",
            .surface = std::string{utl::split_front(line, '\t')},
            .pos1 = std::string{utl::split_front(line, ',')},
            .pos2 = std::string{utl::split_front(line, ',')},
            .pos3 = std::string{utl::split_front(line, ',')},
            .pos4 = std::string{utl::split_front(line, ',')},
            .pos5 = std::string{utl::split_front(line, ',')},
            .pos6 = std::string{utl::split_front(line, '\t')},

            .normalized_form = std::string{utl::split_front(line, '\t')},
            .dictionary_form = std::string{utl::split_front(line, '\t')},
            .reading = std::string{utl::split_front(line, '\t')},
            .dictionary_id = std::stoi(std::string{utl::split_front(line, '\t')}),
            .extra_info = std::string{line},
    };
}

auto Sudachi::isCovered(const std::vector<SudachiToken>& tokens, const std::string& text) -> bool
{
    // return std::ranges::fold_left(parts,
    //                               std::pair{s, size_t{0}},
    //                               [](auto acc, auto p) {
    //                                   return acc.first.starts_with(p, acc.second)
    //                                                  ? std::pair{acc.first, acc.second + p.size()}
    //                                                  : std::pair{std::string_view{}, size_t{0}};
    //                               })
    //                .second
    //        == s.size();
    auto cutText = std::ranges::fold_left(tokens, std::string_view{text},
                                          [&text](std::string_view cutText, const SudachiToken token) -> std::string_view {
                                              if (auto pos = cutText.find(token.surface); pos != cutText.npos) {
                                                  return cutText.substr(pos + token.surface.length());
                                              }
                                              spdlog::error("NOT_FOUND: {} - {} ::: {}", token.surface, cutText, text);
                                              return {"NOT_FOUND"};
                                          });
    // if (cutText == "NOT_FOUND") {
    //     std::ranges::fold_left(tokens, std::string_view{text},
    //                            [&text](std::string_view cutText, const SudachiToken token) -> std::string_view {
    //                                if (auto pos = cutText.find(token.surface); pos != cutText.npos) {
    //                                    spdlog::warn("{} - {} ::: {}", token.surface, cutText, text);
    //                                    return cutText.substr(pos + token.surface.length());
    //                                }
    //                                spdlog::error("NOT_FOUND: {} - {} ::: {}", token.surface, cutText, text);
    //                                return {"NOT_FOUND"};
    //                            });
    // }

    // if (!cutText.empty()) {
    //     spdlog::warn("\"{}\"", cutText);
    // }
    return cutText.empty();
}

} // namespace annotation
