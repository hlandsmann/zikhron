#include "TextCard.h"
#include <algorithm>
#include <fstream>
#include "rapidjosnWrapper.h"

void CardDB::loadFromSingleJson(std::string jsonFileName) {
    std::ifstream             cardFile(jsonFileName);
    rapidjson::IStreamWrapper isw(cardFile);

    rapidjson::Document json;
    json.ParseStream(isw);

    for (const auto &jsonCard : json.GetArray()) {
        if (jsonCard.HasMember("dlg")) {
            const auto &dlg = jsonCard["dlg"];
            if (not dlg.IsArray())
                continue;

            auto dialogueCard = std::make_unique<DialogueCard>();
            for (const auto &speakerTextPair : dlg.GetArray()) {
                if (not speakerTextPair.MemberCount())
                    continue;
                const auto &item = *speakerTextPair.MemberBegin();
                dialogueCard->dialogue.push_back({icu::UnicodeString::fromUTF8(item.name.GetString()),
                                                  icu::UnicodeString::fromUTF8(item.value.GetString())});
            }
            dialogueCard->dialogue.shrink_to_fit();
            cards.push_back(std::move(dialogueCard));
        }
        if (jsonCard.HasMember("text")) {
            const auto &text = jsonCard["text"];
            auto textCard = std::make_unique<TextCard>();
            textCard->text = icu::UnicodeString::fromUTF8(text.GetString());

            cards.push_back(std::move(textCard));
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
