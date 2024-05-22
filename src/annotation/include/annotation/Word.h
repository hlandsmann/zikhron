#pragma once
#include <dictionary/ZH_Dictionary.h>
#include <misc/Identifier.h>

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
class VocableProgress;

namespace annotation {

struct Option;

class Word
{
public:
    Word(const Word&) = default;
    Word(Word&&) = default;
    virtual ~Word() = default;
    auto operator=(const Word&) -> Word& = default;
    auto operator=(Word&&) -> Word& = default;

    Word(std::string_view description, VocableId vocableId, const std::shared_ptr<ZH_Dictionary>& dictionary);
    Word(std::vector<ZH_Dictionary::Entry>&& dictionaryEntries, VocableId vocableId);
    [[nodiscard]] auto serialize() const -> std::string;
    [[nodiscard]] auto getId() const -> VocableId;
    [[nodiscard]] auto Key() const -> std::string;
    [[nodiscard]] auto getProgress() const -> std::shared_ptr<VocableProgress>;
    [[nodiscard]] auto getOptions() const -> const std::vector<Option>&;
    [[nodiscard]] auto isConfigureable() const -> bool;

private:
    void parseOptions(std::string_view description);
    VocableId vocableId{};
    std::shared_ptr<VocableProgress> vocableProgress;

    std::string key;
    std::vector<Option> options;
    std::string pronounciation;
    std::vector<std::string> meanings;

    std::vector<ZH_Dictionary::Entry> dictionaryEntries;
};

struct Option
{
    Option() = default;
    Option(std::string_view description);
    [[nodiscard]] auto serialize() const -> std::string;

    std::string pronounciation;
    std::vector<std::string> meanings;
};

} // namespace annotation
