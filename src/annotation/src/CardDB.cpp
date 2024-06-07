#include "CardDB.h"

#include <Card.h>
#include <Tokenizer.h>
#include <WordDB.h>
#include <dictionary/ZH_Dictionary.h>
#include <fmt/format.h>
#include <misc/Config.h>
#include <misc/Identifier.h>
#include <spdlog/spdlog.h>
#include <unicode/unistr.h>
#include <utils/StringU8.h>

#include <ctre.hpp>
#include <exception>
#include <filesystem>
#include <format>
#include <fstream>
#include <ios>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <sys/types.h>

namespace annotation {
auto cardFromJsonFile(
        const std::string& filename,
        CardId cardId,
        const std::shared_ptr<WordDB>& wordDB,
        const std::shared_ptr<annotation::Tokenizer>& tokenizer

        ) -> std::unique_ptr<Card>
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
        auto dialogue = std::vector<DialogueCard::DialogueItem>{};
        for (const auto& speakerTextPair : dlg) {
            if (speakerTextPair.empty()) {
                continue;
            }
            const auto& item = *speakerTextPair.items().begin();
            dialogue.emplace_back(icu::UnicodeString::fromUTF8(std::string(item.key())),
                                  icu::UnicodeString::fromUTF8(std::string(item.value())));
        }
        dialogue.shrink_to_fit();
        auto dialogueCard = std::make_unique<DialogueCard>(filename, cardId, wordDB, tokenizer, std::move(dialogue));
        return dialogueCard;
    }
    if (jsonCard["type"] == "text") {
        auto text = icu::UnicodeString::fromUTF8(std::string(jsonCard["content"]));
        auto textCard = std::make_unique<TextCard>(filename, cardId, wordDB, tokenizer, std::move(text));
        return textCard;
    }
    throw std::runtime_error("Invalid file format for card json-file");
}

CardDB::CardDB(std::shared_ptr<zikhron::Config> _config, std::shared_ptr<WordDB> _wordDB)
    : config{std::move(_config)}
    , wordDB{std::move(_wordDB)}

    , tokenizer{std::make_shared<annotation::Tokenizer>(config, wordDB)}
    , cards{loadFromDirectory(config->DatabaseDirectory() / s_cardSubdirectory, wordDB, tokenizer)}
{}

auto CardDB::loadFromDirectory(const std::filesystem::path& directoryPath,
                               const std::shared_ptr<WordDB>& wordDB,
                               const std::shared_ptr<annotation::Tokenizer>& tokenizer)
        -> std::map<CardId, CardPtr>
{
    std::map<CardId, CardPtr> cards;
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
            cards[cardId] = cardFromJsonFile(entry, cardId, wordDB, tokenizer);
        } catch (std::exception& e) {
            spdlog::error("{} - file: {}", e.what(), entry.filename().string());
        }
    }
    return cards;
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

auto CardDB::getAnnotationAlternativesForCard(CardId cardId) const -> std::vector<Alternative>
{
    return cards.at(cardId)->getAlternatives();
}
} // namespace annotation
