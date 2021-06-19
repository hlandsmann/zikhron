#pragma once

#include <QGuiApplication>
#include <QQuickItem>

// #include <TextCard.h>

#include <utils/Markup.h>
// #include <unicode/unistr.h>
#include <utils/StringU8.h>
// #include <QGuiApplication>
// #include <QMouseEvent>
// #include <QQuickItem>
// #include <QSharedPointer>
#include <ZH_Annotator.h>
#include <ZH_Dictionary.h>

#include "DataThread.h"

namespace card {
class Annotate : public QQuickItem {
    Q_OBJECT
public:
    Annotate();

public slots:
    void hoveredTextPosition(int pos);
    void clickedTextPosition(int pos);
    void chosenAnnotation(int index);
    void getAnnotation(const PtrParagraph &);

signals:
    void textUpdate(QString newText);
    void annotationPossibilities(QList<QString> marked, QList<QString> unmarked, int pos);

private:
    auto childMouseEventFilter(QQuickItem *, QEvent *event) -> bool override;
    void useCard();
    QSharedPointer<markup::Paragraph> paragraph;

    std::vector<std::vector<int>> combinations;
    std::vector<utl::ItemU8> characters;
    // QSharedPointer<ZH_Dictionary> zh_dict;
    // QSharedPointer<Card> ptrCard;
    // std::unique_ptr<ZH_Annotator> zh_annotator;
};

}  // namespace card
