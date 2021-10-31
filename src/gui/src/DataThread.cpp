#include "DataThread.h"
#include <annotation/TextCard.h>
#include <spaced_repetition/VocabularySR.h>
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
    vocabularySR = std::make_unique<VocabularySR>(std::move(cardDB), zh_dict);

    qDebug() << "Created Dictionary";
    auto cardInformation = vocabularySR->getCard();
    sendActiveCard(std::move(cardInformation));

}

void DataThread::getCardEase(QList<int> ease) {
    std::vector<Ease> easeWork;
    ranges::transform(ease, std::back_inserter(easeWork), mapIntToEase);
    vocabularySR->setEaseLastCard(paragraph->getRestoredOrderOfEaseList(easeWork));

    auto cardInformation = vocabularySR->getCard();
    sendActiveCard(std::move(cardInformation));
}

void DataThread::cardAnnotationChoice(QList<int> qtCombination, QList<QString> qtCharacterSequence) {
    std::vector<int> combination;
    std::vector<utl::ItemU8> characterSequence;
    ranges::copy(qtCombination, std::back_inserter(combination));
    ranges::transform(qtCharacterSequence, std::back_inserter(characterSequence), &QString::toStdString);

    auto cardInformation = vocabularySR->addAnnotation(combination, characterSequence);
    sendActiveCard(std::move(cardInformation));
}

void DataThread::sendActiveCard(CardInformation&& cardInformation) {
    auto [current_card, vocables, ease] = std::move(cardInformation);
    auto current_card_clone = std::unique_ptr<Card>(current_card->clone());
    paragraph = QSharedPointer<markup::Paragraph>::create(std::move(current_card));
    paragraph->setupVocables(std::move(vocables));
    auto orderedEase = paragraph->getRelativeOrderedEaseList(ease);

    QList<int> vocEaseList;
    ranges::transform(
        orderedEase, std::back_inserter(vocEaseList), mapEaseToInt, [](const auto& id_ease) {
            return id_ease.second;
        });
    emit sendParagraph(paragraph, vocEaseList);
    annotation = QSharedPointer<markup::Paragraph>::create(std::move(current_card_clone));
    emit sendAnnotation(annotation);
}
