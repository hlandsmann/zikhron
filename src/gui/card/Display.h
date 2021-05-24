#pragma once

#include <Ease.h>
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
    void getParagraph(const PtrParagraph &, const QList<int> &);
    void clickedEase(QList<int>);

signals:
    void hovered(int x, int y);
    void clicked(int x, int y);
    void textUpdate(QString newText);
    void vocableUpdate(QString newVocables, QList<int> vocablePosList, QList<int> vocableEaseList);
    void openPopup(int pos, QString popupText, QList<int> popupPosList);

    void doubleClicked();
    void sendEase(QList<int>);

private:
    auto childMouseEventFilter(QQuickItem *, QEvent *event) -> bool override;

    int lastPos = -1;
    QSharedPointer<markup::Paragraph> paragraph;
    std::string annotated;
    std::unique_ptr<ZH_Annotator> zh_annotator;

    struct AnnotatedBlock {
        std::unique_ptr<ZH_Annotator> zh_annotator;
        std::unique_ptr<markup::Paragraph> paragraph;
    };
};
}  // namespace card
