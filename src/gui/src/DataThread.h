#pragma once

#include <memory>
#include <QSharedPointer>
#include <QThread>
#include <annotation/Markup.h>
#include <annotation/Ease.h>
#include <dictionary/ZH_Dictionary.h>

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
    void cardAnnotationChoice(QList<int> qtCombination, QList<QString> qtCharacterSequence);
signals:
    void sendDictionary(const PtrDictionary&);
    void sendCard(const PtrCard&);
    void sendParagraph(const PtrParagraph&, const QList<int>&);
    void sendAnnotation(const PtrParagraph&);


protected:
    void run() override;

private:
    using Item_Id_vt = std::vector<std::pair<ZH_Dictionary::Item, uint>>;
    using Id_Ease_vt = std::map<uint, Ease>;
    using CardInformation = std::tuple<std::unique_ptr<Card>, Item_Id_vt, Id_Ease_vt>;

    void sendActiveCard(CardInformation&&);
    std::unique_ptr<VocabularySR> vocabularySR;
    std::shared_ptr<ZH_Dictionary> zh_dict;
    QSharedPointer<markup::Paragraph> paragraph;
    QSharedPointer<markup::Paragraph> annotation;
};

Q_DECLARE_METATYPE(PtrDictionary)
Q_DECLARE_METATYPE(PtrCard)
Q_DECLARE_METATYPE(PtrParagraph)