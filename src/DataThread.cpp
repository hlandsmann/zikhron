#include "DataThread.h"
#include <TextCard.h>
#include <VocabularySR.h>
#include <utils/StringU8.h>
#include <QDebug>
#include <algorithm>
#include <iostream>
#include <ranges>

namespace ranges = std::ranges;
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
    zh_dict = std::make_shared<ZH_Dictionary>("/home/harmen/src/zikhron/dictionaries/cedict_ts.u8");
    // auto zh_dict2 = QSharedPointer<ZH_Dictionary>::create("../dictionaries/cedict_ts.u8");
    vocabularySR = std::make_unique<VocabularySR>(std::move(cardDB), zh_dict);

    // auto zh_dict = QSharedPointer<ZH_Dictionary>::create("../dictionaries/handedict.u8");
    qDebug() << "Created Dictionary";
    auto cardInformation = vocabularySR->getCard();
    sendActiveCard(std::move(cardInformation));
    // emit sendDictionary(zh_dict2);
    // emit sendCard(long_card);
}

void DataThread::getCardEase(QList<int> ease) {
    std::vector<Ease> easeWork;
    ranges::transform(ease, std::back_inserter(easeWork), mapIntToEase);
    vocabularySR->setEaseLastCard(paragraph->getRestoredOrderOfEaseList(easeWork));

    auto cardInformation = vocabularySR->getCard();
    sendActiveCard(std::move(cardInformation));
}

void DataThread::cardAnnotationChoice(QList<int> qtCombination, QList<QString> qtCharacters) {
    std::vector<int> combination;
    std::vector<utl::ItemU8> characters;
    ranges::copy(qtCombination, std::back_inserter(combination));
    ranges::transform(qtCharacters, std::back_inserter(characters), &QString::toStdString);

    auto cardInformation = vocabularySR->addAnnotation(combination, characters);
    sendActiveCard(std::move(cardInformation));
}

void DataThread::sendActiveCard(CardInformation&& cardInformation) {
    auto [current_card, vocables, ease] = std::move(cardInformation);
    paragraph = QSharedPointer<markup::Paragraph>::create(*current_card, zh_dict);
    paragraph->setupVocables(std::move(vocables));
    auto orderedEase = paragraph->getRelativeOrderedEaseList(ease);

    QList<int> vocEaseList;
    ranges::transform(
        orderedEase, std::back_inserter(vocEaseList), mapEaseToInt, [](const auto& id_ease) {
            return id_ease.second;
        });
    emit sendParagraph(paragraph, vocEaseList);

    annotation = QSharedPointer<markup::Paragraph>::create(*current_card, zh_dict);
    emit sendAnnotation(annotation);
}
