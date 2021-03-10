#include "DataThread.h"
#include <utils/StringU8.h>
#include <QDebug>
#include <iostream>

namespace {

auto loadCardDB() -> CardDB {
    CardDB cardDB;
    try {
        cardDB.loadFromDirectory("/home/harmen/src/zikhron/conversion/cards");
    } catch (const std::exception& e) { std::cout << e.what() << std::endl; } catch (...) {
        std::cout << "Unknown Error" << std::endl;
    }
    return cardDB;
}

auto getLongestCard(CardDB& cardDB) -> QSharedPointer<Card> {
    icu::UnicodeString maxText = "";
    auto ltext = [](const std::vector<icu::UnicodeString>& vec) -> int {
        if (vec.empty())
            return 0;
        return std::max_element(vec.begin(),
                                vec.end(),
                                [](const auto& t1, const auto& t2) { return t1.length() < t2.length(); })
            ->length();
    };
    std::cout << "Card DB size: " << cardDB.get().size() << "\n";
    const auto it = std::max_element(
        cardDB.get().begin(), cardDB.get().end(), [ltext](const auto& card1, const auto& card2) {
            std::vector<icu::UnicodeString> vec1 = card1->getTextVector();
            std::vector<icu::UnicodeString> vec2 = card2->getTextVector();

            return ltext(vec1) < ltext(vec2);
            // return vec1.size() < vec2.size();
        });

    if (TextCard* card = dynamic_cast<TextCard*>((*it).get()); card != nullptr)
        return QSharedPointer<TextCard>::create(*card);
    if (DialogueCard* card = dynamic_cast<DialogueCard*>((*it).get()); card != nullptr)
        return QSharedPointer<DialogueCard>::create(*card);

    return QSharedPointer<TextCard>::create();
}

}  // namespace

void DataThread::run() {
    if (cardDB.get().empty())
        cardDB = loadCardDB();
    auto zh_dict = std::make_shared<ZH_Dictionary>("../dictionaries/cedict_ts.u8");
    auto zh_dict2 = QSharedPointer<ZH_Dictionary>::create("../dictionaries/cedict_ts.u8");

    // auto zh_dict = QSharedPointer<ZH_Dictionary>::create("../dictionaries/handedict.u8");
    qDebug() << "Created Dictionary";

    auto long_card = getLongestCard(cardDB);
    auto paragraph = QSharedPointer<markup::Paragraph>::create(*long_card, zh_dict);

    emit sendParagraph(paragraph);
    emit sendDictionary(zh_dict2);
    emit sendCard(long_card);
}
