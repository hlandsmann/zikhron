#include "TextCard.h"
#include <fmt/format.h>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include "rapidjosnWrapper.h"

#include <iostream>

namespace {
auto cardFromJsonFile(const std::string &filename) -> std::unique_ptr<Card> {
    std::ifstream cardFile(filename, std::ios::in | std::ios::binary);
    if (!cardFile)
        throw std::runtime_error("Failure to read file");

    rapidjson::IStreamWrapper isw(cardFile);
    rapidjson::Document jsonCard;
    jsonCard.ParseStream(isw);

    if (const auto &version = jsonCard["version"]; version != "0.0")
        throw std::runtime_error(
            fmt::format("Supported version is '0.0', but got '{}'", version.GetString()));

    if (jsonCard["type"] == "dialogue") {
        const auto &dlg = jsonCard["content"];
        if (not dlg.IsArray())
            throw std::runtime_error("Expected an array");

        auto dialogueCard = std::make_unique<DialogueCard>();
        for (const auto &speakerTextPair : dlg.GetArray()) {
            if (not speakerTextPair.MemberCount())
                continue;
            const auto &item = *speakerTextPair.MemberBegin();
            dialogueCard->dialogue.push_back({icu::UnicodeString::fromUTF8(item.name.GetString()),
                                              icu::UnicodeString::fromUTF8(item.value.GetString())});
        }
        dialogueCard->dialogue.shrink_to_fit();
        return dialogueCard;
    }
    if (jsonCard["type"] == "text") {
        const auto &text = jsonCard["content"];
        auto textCard = std::make_unique<TextCard>();
        textCard->text = icu::UnicodeString::fromUTF8(text.GetString());

        return textCard;
    }
    throw std::runtime_error("Invalid file format for card json-file");
}
}  // namespace

void CardDB::loadFromDirectory(std::string directoryPath) {
    namespace fs = std::filesystem;

    for (const fs::path entry : fs::directory_iterator(directoryPath)) {
        try {
            cards.push_back(cardFromJsonFile(entry));
        } catch (std::exception &e) {
            std::cout << e.what() << " - file: " << entry.filename() << std::endl;
        }
    }
}

auto DialogueCard::getTextVector() const -> std::vector<icu::UnicodeString> {
    std::vector<icu::UnicodeString> textVector;
    textVector.reserve(dialogue.size());
    std::transform(
        dialogue.begin(), dialogue.end(), std::back_inserter(textVector), [](const auto &item) {
            return item.text;
        });
    return textVector;
}
auto TextCard::getTextVector() const -> std::vector<icu::UnicodeString> { return {text}; }

auto CardDB::get() const -> const std::vector<CardPtr> & { return cards; }
