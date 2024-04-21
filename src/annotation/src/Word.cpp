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

namespace annotation {
Word::Word(std::shared_ptr<VocableProgress> _vocableProgress,
           VocableId vocId,
           const std::shared_ptr<ZH_Dictionary>& dictionary)
    : vocableProgress{std::move(_vocableProgress)}
    , dictionaryPos{static_cast<std::size_t>(vocId)}
{
    auto entry = dictionary->EntryFromPosition(vocId);
    key = std::move(entry.key);
    pronounciation = std::move(entry.pronounciation);
    meanings.emplace_back(std::move(entry.meanings.front()));
    dictionaryEntries = dictionary->EntryVectorFromKey(key);
}

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

} // namespace annotation
