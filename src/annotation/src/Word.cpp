#include "annotation/Word.h"

#include <dictionary/ZH_Dictionary.h>
#include <fmt/format.h>
#include <misc/Identifier.h>
#include <spaced_repetition/VocableProgress.h>
#include <utils/spdlog.h>
#include <utils/string_split.h>

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace annotation {

Word::Word(const std::string& description, const std::shared_ptr<ZH_Dictionary>& dictionary)
{
    auto rest = std::string_view{description};
    key = utl::split_front(rest, ';');
    dictionaryPos = std::stoul(std::string{utl::split_front(rest, ';')});
    vocableProgress = std::make_shared<VocableProgress>(utl::split_front(rest, ';'));
    pronounciation = utl::split_front(rest, ';');
    while (true) {
        auto meaning = std::string{utl::split_front(rest, '/')};
        if (meaning.empty()) {
            break;
        }
        meanings.push_back(meaning);
    }

    // spdlog::info("{};{};{}", key, dictionaryPos, vocableProgress->serialize());
}

Word::Word(std::vector<ZH_Dictionary::Entry>&& _dictionaryEntries)
    : vocableProgress{std::make_shared<VocableProgress>()}
    , dictionaryEntries{std::move(_dictionaryEntries)}
{
    key = dictionaryEntries.front().key;
    pronounciation = dictionaryEntries.front().pronounciation;
    meanings.push_back(dictionaryEntries.front().meanings.front());
    dictionaryPos = dictionaryEntries.front().id;
}

auto Word::serialize() const -> std::string
{
    return fmt::format("{};{};{};{};{}/\n", key,
                       dictionaryPos,
                       vocableProgress->serialize(),
                       pronounciation,
                       fmt::join(meanings, "/"));
}

auto Word::getId() const -> VocableId
{
    return vocableId;
}

auto Word::Key() const -> std::string
{
    return key;
}

} // namespace annotation
