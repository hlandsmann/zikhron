#pragma once

#include <QtCore/qsharedpointer.h>  // for QSharedPointer
#include <qlist.h>                  // for QList
#include <qobjectdefs.h>            // for Q_OBJECT, signals, slots
#include <qquickitem.h>             // for QQuickItem
#include <qstring.h>                // for QString
#include <utils/StringU8.h>         // for ItemU8
#include <vector>                   // for vector
#include "DataThread.h"             // for PtrParagraph
class QObject;
namespace markup { class Paragraph; }

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
