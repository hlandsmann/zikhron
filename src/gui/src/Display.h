#pragma once

#include <QtCore/qsharedpointer.h>
#include <qlist.h>
#include <qobjectdefs.h>
#include <qquickitem.h>
#include <qstring.h>

#include <DataThread.h>
#include <annotation/Markup.h>
#include <annotation/ZH_Annotator.h>
#include <memory>
#include <string>

class QEvent;
class QObject;

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
    QSharedPointer<ZH_Annotator> zh_annotator;

    // struct AnnotatedBlock {
    //     std::unique_ptr<ZH_Annotator> zh_annotator;
    //     std::unique_ptr<markup::Paragraph> paragraph;
    // };
};
}  // namespace card