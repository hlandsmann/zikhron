#pragma once

#include <TextCard.h>
#include <utils/Markup.h>
#include <unicode/unistr.h>
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

    auto childMouseEventFilter(QQuickItem *, QEvent *event) -> bool override;
public slots:
    void hoveredTextPosition(int pos);
    void getDictionary(const PtrDictionary &zh_dict);
    void getCard(const PtrCard &ptrCard);

signals:
    void hovered(int x, int y);
    void clicked(int x, int y);
    void textUpdate(QString newText);
    void doubleClicked();

private:
    // auto getLongText() const -> utl::StringU8;
    void useCard();

    int lastPos = -1;
    markup::Paragraph paragraph;
    QSharedPointer<ZH_Dictionary> zh_dict;
    QSharedPointer<Card> ptrCard;
    std::string annotated;
};
}  // namespace card
