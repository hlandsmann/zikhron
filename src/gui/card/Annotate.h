#pragma once

#include <QGuiApplication>
#include <QQuickItem>
#include <utils/Markup.h>
#include <utils/StringU8.h>
#include <ZH_Annotator.h>
#include <ZH_Dictionary.h>

#include "DataThread.h"

namespace card {
class Annotate : public QQuickItem {
    Q_OBJECT
public:
    Annotate() = default;

public slots:
    void hoveredTextPosition(int pos);
    void clickedTextPosition(int pos);
    void chosenAnnotation(int index);
    void getAnnotation(const PtrParagraph &);

signals:
    void textUpdate(QString newText);
    void annotationPossibilities(QList<QString> marked, QList<QString> unmarked, int pos);
    void cardAnnotationChoice(QList<int> combination, QList<QString> characterSequence);
private:
    QSharedPointer<markup::Paragraph> paragraph;

    std::vector<std::vector<int>> combinations;
    std::vector<utl::ItemU8> characterSequence;
};

}  // namespace card
