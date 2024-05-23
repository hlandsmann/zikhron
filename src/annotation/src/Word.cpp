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

Word::Word(std::string_view description, VocableId _vocableId, const std::shared_ptr<ZH_Dictionary>& dictionary)
    : vocableId{_vocableId}
{
    auto rest = std::string_view{description};
    key = utl::split_front(rest, ';');
    dictionaryEntries = dictionary->entryVectorFromKey(key);
    vocableProgress = std::make_shared<VocableProgress>(utl::split_front(rest, ';'));

    parseOptions(rest);

    // spdlog::info("{};{};{}", key, dictionaryPos, vocableProgress->serialize());
}

Word::Word(std::vector<ZH_Dictionary::Entry>&& _dictionaryEntries, VocableId _vocableId)
    : vocableId{_vocableId}
    , vocableProgress{std::make_shared<VocableProgress>()}
    , dictionaryEntries{std::move(_dictionaryEntries)}
{
    key = dictionaryEntries.front().key;
    pronounciation = dictionaryEntries.front().pronounciation;
    meanings.push_back(dictionaryEntries.front().meanings.front());
    vocableProgress = std::make_shared<VocableProgress>(VocableProgress::new_vocable);
}

auto Word::serialize() const -> std::string
{
    return fmt::format("{};{};{};{}/\n", key,
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

auto Word::getProgress() const -> std::shared_ptr<VocableProgress>
{
    return vocableProgress;
}

auto Word::getDefinitions() const -> const std::vector<Definition>&
{
    return options;
}

auto Word::isConfigureable() const -> bool
{
    return ((!dictionaryEntries.empty())
            && (dictionaryEntries.front().meanings.size() > 1 || dictionaryEntries.size() > 1));
}

auto Word::getDictionaryEntries() const -> const std::vector<ZH_Dictionary::Entry>&
{
    return dictionaryEntries;
}

void Word::parseOptions(std::string_view description)
{
    auto rest = std::string_view{description};
    while (true) {
        auto optionSV = utl::split_front(rest, '\\');
        if (optionSV.empty()) {
            break;
        }
        options.emplace_back(optionSV);
    }
}

Definition::Definition(std::string_view description)
{
    auto rest = std::string_view{description};
    pronounciation = utl::split_front(rest, ';');
    while (true) {
        auto meaning = std::string{utl::split_front(rest, '/')};
        if (meaning.empty()) {
            break;
        }
        meanings.push_back(meaning);
    }
}
} // namespace annotation
