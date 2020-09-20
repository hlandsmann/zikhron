#pragma once

#include <TextCard.h>
#include <unicode/unistr.h>
#include <QGuiApplication>
#include <QMouseEvent>
#include <QQuickItem>
#include <QSharedPointer>
#include <utils/StringU8.h>
#include "DataThread.h"

class Observer : public QQuickItem {
    Q_OBJECT

public:


    Observer() {
        setFiltersChildMouseEvents(true);
        // connect(sender(), &QObject::destroyed, this, &Observer::hoveredTextPosition);
    }

    auto childMouseEventFilter(QQuickItem *, QEvent *event) -> bool override;
public slots:
    void hoveredTextPosition(int pos);
    void getDictionary(const ptrDictionary &zh_dict);
    void getDic() { qDebug() << " Dic received "; }

signals:
    void hovered(int x, int y);
    void clicked();
    void textUpdate(QString newText);
    void doubleClicked();

private:
    auto getLongText() const -> utl::StringU8;

    mutable CardDB cardDB;

    QSharedPointer<ZH_Dictionary> zh_dict;
    std::string annotated;
};
