#pragma once

#include <TextCard.h>
#include <gui/HtmlGen.h>
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
    Display() {
        setFiltersChildMouseEvents(true);
        // connect(sender(), &QObject::destroyed, this, &Display::hoveredTextPosition);
    }

    auto childMouseEventFilter(QQuickItem *, QEvent *event) -> bool override;
public slots:
    void hoveredTextPosition(int pos);
    void getDictionary(const ptrDictionary &zh_dict);
    void getDic() { qDebug() << " Dic received "; }

signals:
    void hovered(int x, int y);
    void clicked(int x, int y);
    void textUpdate(QString newText);
    void doubleClicked();

private:
    auto getLongText() const -> utl::StringU8;

    mutable CardDB cardDB;

    int lastPos = -1;
    markup::Paragraph paragraph;
    QSharedPointer<ZH_Dictionary> zh_dict;
    std::string annotated;
};
}  // namespace card
