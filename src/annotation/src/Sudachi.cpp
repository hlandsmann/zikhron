#include "Sudachi.h"

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

void Sudachi::split(const std::string& text)
{
    const std::string fwdString = fmt::format("{}\n", text);
    processPipe->write(fwdString);
    // bool print = true;
    // spdlog::info("\n{}", text);

    // spdlog::info("\n{}", chunk);
    std::vector<SudachiToken> tokens;
    while (true) {
        const auto& chunk = processPipe->getChunk();
        if (chunk.empty()) {
            break;
        }
        fmt::print("\n{}", chunk);
        auto chunkView = std::string_view{chunk};
        while (true) {
            auto line = utl::split_front(chunkView, '\n');
            if (line.empty() || line == "EOS") {
                break;
            }
            tokens.push_back(parseLine(line));
            spdlog::info("{} - {}", tokens.back().surface, tokens.back().extra_info);
        }
        if (isCovered(tokens, text)) {
            break;
        }
        // fmt::print("\"{}\"", text);
        fmt::print("{}", chunk);
    }
    for(const auto& token: tokens){

            spdlog::info("{} - {}", token.surface, token.extra_info);
    }

    // while (true) {
    //     const auto& chunk2 = processPipe->getChunk();
    //     if (chunk2.empty()) {
    //         break;
    //     }
    //     if (print) {
    //         print = false;
    //
    //         spdlog::info(text);
    //         fmt::print("\n{}", chunk);
    //     }
    //     fmt::print("\n{}", chunk2);
    // }
}

auto Sudachi::parseLine(std::string_view line) -> SudachiToken
{
    return {
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
    if (cutText == "NOT_FOUND") {
        std::ranges::fold_left(tokens, std::string_view{text},
                               [&text](std::string_view cutText, const SudachiToken token) -> std::string_view {
                                   if (auto pos = cutText.find(token.surface); pos != cutText.npos) {
                                       spdlog::warn("{} - {} ::: {}", token.surface, cutText, text);
                                       return cutText.substr(pos + token.surface.length());
                                   }
                                   spdlog::error("NOT_FOUND: {} - {} ::: {}", token.surface, cutText, text);
                                   return {"NOT_FOUND"};
                               });
    }

    // if (!cutText.empty()) {
    //     spdlog::warn("\"{}\"", cutText);
    // }
    return cutText.empty();
}

} // namespace annotation
