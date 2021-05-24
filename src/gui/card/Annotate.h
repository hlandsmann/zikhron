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
    void getParagraph(const PtrParagraph &);

signals:
    void textUpdate(QString newText);

private:
    auto childMouseEventFilter(QQuickItem *, QEvent *event) -> bool override;
    void useCard();
        QSharedPointer<markup::Paragraph> paragraph;
    // QSharedPointer<ZH_Dictionary> zh_dict;
    // QSharedPointer<Card> ptrCard;
    // std::unique_ptr<ZH_Annotator> zh_annotator;
};

}  // namespace card
