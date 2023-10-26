#include <CardDB.h>
#include <ZH_Annotator.h>
#include <dictionary/ZH_Dictionary.h>
#include <fmt/format.h>
#include <misc/Identifier.h>
#include <spdlog/spdlog.h>
#include <unicode/unistr.h>
#include <utils/StringU8.h>

#include <algorithm>
#include <ctre.hpp>
#include <exception>
#include <filesystem>
#include <format>
#include <fstream>
#include <ios>
#include <iterator>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <stdexcept>
#include <string>
#include <vector>

#include <sys/types.h>
namespace {
auto cardFromJsonFile(const std::string& filename, CardId cardId) -> std::unique_ptr<Card>
{
    std::ifstream cardFile(filename, std::ios::in | std::ios::binary);
    if (!cardFile) {
        throw std::runtime_error("Failure to read file");
    }
    auto jsonCard = nlohmann::json::parse(cardFile);
    if (const auto& version = jsonCard.at("version"); version != "0.0") {
        throw std::runtime_error(std::format("Supported version is '0.0', but got '{}'", std::string(version)));
    }

    if (jsonCard.at("type") == "dialogue") {
        const auto& dlg = jsonCard["content"];
        if (not dlg.is_array()) {
            throw std::runtime_error("Expected an array");
        }
        auto dialogueCard = std::make_unique<DialogueCard>(filename, cardId);
        for (const auto& speakerTextPair : dlg) {
            if (speakerTextPair.empty()) {
                continue;
            }
            const auto& item = *speakerTextPair.items().begin();
            dialogueCard->dialogue.emplace_back(icu::UnicodeString::fromUTF8(std::string(item.key())),
                                                icu::UnicodeString::fromUTF8(std::string(item.value())));
        }
        dialogueCard->dialogue.shrink_to_fit();
        return dialogueCard;
    }
    if (jsonCard["type"] == "text") {
        const auto& text = jsonCard["content"];
        auto textCard = std::make_unique<TextCard>(filename, cardId);
        textCard->text = icu::UnicodeString::fromUTF8(std::string(text));

        return textCard;
    }
    throw std::runtime_error("Invalid file format for card json-file");
}
} // namespace

auto Card::createAnnotator(const std::shared_ptr<const ZH_Dictionary>& _dictionary,
                           const std::map<CharacterSequence, Combination>& _choices) -> ZH_Annotator&
{
    zh_annotator.emplace(getText(), _dictionary, _choices);
    return zh_annotator.value();
}

auto Card::getAnnotator() -> ZH_Annotator&
{
    if (not zh_annotator.has_value()) {
        zh_annotator.emplace();
    }
    return zh_annotator.value();
}

auto Card::Id() const -> CardId
{
    return id;
}

auto DialogueCard::getTextVector() const -> std::vector<icu::UnicodeString>
{
    std::vector<icu::UnicodeString> textVector;
    textVector.reserve(dialogue.size());
    std::transform(
            dialogue.begin(), dialogue.end(), std::back_inserter(textVector), [](const auto& item) {
                return item.text;
            });
    return textVector;
}

auto DialogueCard::getText() const -> utl::StringU8
{
    utl::StringU8 result;
    for (const auto& monologue : dialogue) {
        result.append(monologue.speaker);
        result.append(std::string("~"));
        result.append(monologue.text);
        result.append(std::string("~"));
    }
    return result;
}

auto TextCard::getTextVector() const -> std::vector<icu::UnicodeString>
{
    return {text};
}

auto TextCard::getText() const -> utl::StringU8
{
    return {text};
}

CardDB::CardDB(const std::filesystem::path& directoryPath,
               const std::shared_ptr<const ZH_Dictionary>& dictionary,
               const std::map<CharacterSequence, Combination>& choices)
{
    loadFromDirectory(directoryPath);
    for (const auto& [_, card] : cards) {
        card->createAnnotator(dictionary, choices);
    }
}

void CardDB::loadFromDirectory(const std::filesystem::path& directoryPath)
{
    namespace fs = std::filesystem;
    auto card_fn_matcher = ctre::match<"(\\d{6})(_dlg|_text)(\\.json)">;
    for (const fs::path& entry : fs::directory_iterator(directoryPath)) {
        std::string fn = entry.filename().string();
        auto card_fn_match = card_fn_matcher(fn);
        if (not card_fn_match) {
            spdlog::warn("File \"{}\" is ignored / not considered as card", fn);
            continue;
        }
        try {
            auto cardId = static_cast<CardId>(std::stoi(card_fn_match.get<1>().to_string()));
            if (cards.find(cardId) != cards.end()) {
                spdlog::warn("File \"{}\" ignored, because number {} is already in use!",
                             entry.filename().string(),
                             cardId);
                continue;
            }
            cards[cardId] = cardFromJsonFile(entry, cardId);
        } catch (std::exception& e) {
            spdlog::error("{} - file: {}", e.what(), entry.filename().string());
        }
    }
}

auto CardDB::get() const -> const std::map<CardId, CardPtr>&
{
    return cards;
}

auto CardDB::atId(CardId cardId) -> CardPtr&
{
    return cards.at(cardId);
}

auto CardDB::atId(CardId cardId) const -> CardPtrConst
{
    return {cards.at(cardId)};
}
