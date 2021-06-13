#pragma once

#include <memory>
#include <QSharedPointer>
#include <QThread>
#include <utils/Markup.h>
#include <Ease.h>
#include "ZH_Dictionary.h"

class VocabularySR;
class Card;

class PtrDictionary : public QObject {
    Q_OBJECT
public:
    PtrDictionary(QObject* parent = nullptr) { Q_UNUSED(parent); };
    PtrDictionary(const QSharedPointer<ZH_Dictionary>& qsptr) : _qsptr(qsptr){};
    PtrDictionary(const PtrDictionary& other) : QObject(), _qsptr(other.get()){};
    auto get() const -> QSharedPointer<ZH_Dictionary> { return _qsptr; }

private:
    QSharedPointer<ZH_Dictionary> _qsptr;
};

class PtrCard : public QObject {
    Q_OBJECT
public:
    PtrCard(QObject* parent = nullptr) { Q_UNUSED(parent); };
    PtrCard(const QSharedPointer<Card>& qsptr) : _card(qsptr){};
    PtrCard(const PtrCard& other) : QObject(), _card(other.get()){};
    auto get() const -> QSharedPointer<Card> { return _card; }

private:
    QSharedPointer<Card> _card;
};

class PtrParagraph : public QObject {
    Q_OBJECT
public:
    PtrParagraph(QObject* parent = nullptr) { Q_UNUSED(parent); };
    PtrParagraph(const QSharedPointer<markup::Paragraph>& _paragraph) : paragraph(_paragraph){};
    PtrParagraph(const PtrParagraph& other) : QObject(), paragraph(other.get()){};
    auto get() const -> QSharedPointer<markup::Paragraph> { return paragraph; }

private:
    QSharedPointer<markup::Paragraph> paragraph;
};

class DataThread : public QThread {
    Q_OBJECT
public:
    DataThread(QObject* parent = nullptr);
    ~DataThread();
public slots:
    void getCardEase(QList<int> ease);
signals:
    void sendDictionary(const PtrDictionary&);
    void sendCard(const PtrCard&);
    void sendParagraph(const PtrParagraph&, const QList<int>&);
    void sendAnnotation(const PtrParagraph&);


protected:
    void run() override;

private:
    void sendNextCard();

    std::unique_ptr<VocabularySR> vocabularySR;
    std::shared_ptr<ZH_Dictionary> zh_dict;
    QSharedPointer<markup::Paragraph> paragraph;
    QSharedPointer<markup::Paragraph> annotation;
};

Q_DECLARE_METATYPE(PtrDictionary)
Q_DECLARE_METATYPE(PtrCard)
Q_DECLARE_METATYPE(PtrParagraph)
