#include "DataThread.h"
#include <TextCard.h>
#include <VocabularySR.h>
#include <utils/StringU8.h>
#include <QDebug>
#include <iostream>
DataThread::DataThread(QObject* parent) { Q_UNUSED(parent); }
DataThread::~DataThread() = default;
namespace {

auto loadCardDB() -> CardDB {
    CardDB cardDB;
    try {
        cardDB.loadFromDirectory("/home/harmen/src/zikhron/conversion/xxcards");
    } catch (const std::exception& e) { std::cout << e.what() << std::endl; } catch (...) {
        std::cout << "Unknown Error" << std::endl;
    }
    return cardDB;
}

}  // namespace

void DataThread::run() {
    CardDB cardDB = loadCardDB();
    zh_dict = std::make_shared<ZH_Dictionary>("../dictionaries/cedict_ts.u8");
    // auto zh_dict2 = QSharedPointer<ZH_Dictionary>::create("../dictionaries/cedict_ts.u8");
    vocabularySR = std::make_unique<VocabularySR>(std::move(cardDB), zh_dict);

    // auto zh_dict = QSharedPointer<ZH_Dictionary>::create("../dictionaries/handedict.u8");
    qDebug() << "Created Dictionary";
    sendNextCard();
    // emit sendDictionary(zh_dict2);
    // emit sendCard(long_card);
}

void DataThread::getCardEase(Ease ease) {
    vocabularySR->setEaseLastCard(ease);
    sendNextCard();
}

void DataThread::sendNextCard() {
    auto [current_card, vocables] = vocabularySR->getCard();
    auto paragraph = QSharedPointer<markup::Paragraph>::create(*current_card, zh_dict);
    paragraph->setupVocables(std::move(vocables));
    emit sendParagraph(paragraph);
}
