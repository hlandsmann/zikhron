#pragma once

#include <TextCard.h>
#include <ZH_Annotator.h>
#include <ZH_Dictionary.h>
#include <unicode/unistr.h>
#include <utils/Markup.h>
#include <utils/StringU8.h>
#include <QGuiApplication>
#include <QMouseEvent>
#include <QQuickItem>
#include <QSharedPointer>
#include "DataThread.h"

namespace card {
class Display : public QQuickItem {
    Q_OBJECT

public:
    Display();

public slots:
    void hoveredTextPosition(int pos);
    void clickedTextPosition(int pos);
    void getDictionary(const PtrDictionary &);
    void getCard(const PtrCard &);
    void getParagraph(const PtrParagraph &);
    void clickedEase(int ease);

signals:
    void hovered(int x, int y);
    void clicked(int x, int y);
    void textUpdate(QString newText);
    void vocableUpdate(QString newVocables);
    void openPopup(int pos, QString popupText, QList<int> popupPosList);

    void doubleClicked();
    void sendEase(int ease);

private:
    auto childMouseEventFilter(QQuickItem *, QEvent *event) -> bool override;

    // auto getLongText() const -> utl::StringU8;
    void useCard();

    int lastPos = -1;
    QSharedPointer<markup::Paragraph> paragraph;
    QSharedPointer<ZH_Dictionary> zh_dict;
    QSharedPointer<Card> ptrCard;
    std::string annotated;
    std::unique_ptr<ZH_Annotator> zh_annotator;

    struct AnnotatedBlock {
        std::unique_ptr<ZH_Annotator> zh_annotator;
        std::unique_ptr<markup::Paragraph> paragraph;
    };
};
}  // namespace card
