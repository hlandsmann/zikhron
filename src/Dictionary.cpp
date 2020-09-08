#include "Dictionary.h"
#include <unicode/unistr.h>
#include <fstream>
#include <iostream>

#include "rapidjosnWrapper.h"

Dictionary::Word::Word(const Word& word) : key(word.key), value(std::make_unique<WordValue>(*(word.value))) {}
Dictionary::Word::Word(icu::UnicodeString _key, std::unique_ptr<WordValue> _value) : key(_key), value(std::move(_value)) {}

void Dictionary::loadFromJson(std::string jsonFileName) {
    using std::cout;
    using std::endl;

    std::ifstream             dictFile(jsonFileName);
    rapidjson::IStreamWrapper isw(dictFile);

    rapidjson::Document json;
    json.ParseStream(isw);

    words.reserve(json.MemberCount());

    for (const auto& key : RapidMembers(json)) {
        const auto& value = key.value;

        auto wordValue = std::make_unique<WordValue>();
        if (value.HasMember("meaning")) {
            if (const auto& meanings = value["meaning"]; meanings.IsArray()) {
                wordValue->meanings.reserve(meanings.Size());

                for (const auto& meaning : meanings.GetArray()) {
                    wordValue->meanings.push_back(icu::UnicodeString::fromUTF8(meaning.GetString()));
                }
            }
        }

        if (value.HasMember("alt")) {
            if (const auto& alternatives = value["alt"]; alternatives.IsArray()) {
                wordValue->alternatives.reserve(alternatives.Size());

                for (const auto& alternative : alternatives.GetArray()) {
                    wordValue->alternatives.push_back(
                        icu::UnicodeString::fromUTF8(alternative.GetString()));
                }
            }
        }

        if (value.HasMember("pron")) {
            if (const auto& pronounciation = value["pron"]; pronounciation.IsString()) {
                wordValue->pronounciations.push_back(
                    icu::UnicodeString::fromUTF8(pronounciation.GetString()));
                wordValue->pronounciations.shrink_to_fit();
            }
        }
        words.push_back({icu::UnicodeString::fromUTF8(key.name.GetString()), std::move(wordValue)});
    }
    cout << json.MemberCount() << " size: " << words.size() << endl;
}
