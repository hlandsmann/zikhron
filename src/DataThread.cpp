#include "DataThread.h"
#include <utils/StringU8.h>
#include <QDebug>
#include <iostream>

namespace {

/*
{ "id" : "word",
  "pron" : ["ni", "hao"],
  "meaning" : ["first meaning"],
  "selected" : "false",
  "cards" : [{"number" : "quantity"}, {"number" : "quantity"}]
}
*/

/*
{ "id" :

}
*/
// {id, word {} - {{cardNum, quantity}, {cardNum, quantity}}
// id : word : {pron, meaning} - {{cardNum, quantity}, {cardNum, quantity}}

// id, {"debug":"word"}, interval, ease, {{cardnum, when-seen},  {cardnum, when-seen}}

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

    return nullptr;
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

    // utl::StringU8 card_text = markup::Paragraph::textFromCard(*long_card);

    // auto annotator = ZH_Annotator(card_text, zh_dict);

    // for(const auto& item : annotator.UniqueItems())
    //     std::cout << "Word: " << item.text << "\n";
    /* std::set<ZH_Annotator::Item> myDic;
    for (const auto& card : cardDB.get()) {
        utl::StringU8 card_text = markup::Paragraph::textFromCard(*card);
        auto annotator = ZH_Annotator(card_text, zh_dict);

        auto unique = annotator.UniqueItems();
        // for (const auto& item : unique)
        //     std::cout << "Word: " << item.text << "\n";
        std::cout << "size: " << unique.size() << " : ";
        myDic.merge(unique);
        std::cout << unique.size() << "\n";
    }
    std::cout << "Final Size: " << myDic.size() << "\n";
    std::set<utl::ItemU8> allCharacters;

    for (const auto& dicItem : myDic) {
        allCharacters.insert(dicItem.text.cbegin(), dicItem.text.cend());
    }
    for (const auto& mychar : allCharacters) {
        std::cout << mychar;
    }
    std::cout << "\n";
    std::cout << "Count of Characters: " << allCharacters.size() << "\n"; */
    auto paragraph = QSharedPointer<markup::Paragraph>::create(*long_card, zh_dict);

    emit sendParagraph(paragraph);
    emit sendDictionary(zh_dict2);
    emit sendCard(long_card);
}
