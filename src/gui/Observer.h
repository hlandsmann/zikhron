#pragma once

#include <QGuiApplication>
#include <QMouseEvent>
#include <QQuickItem>
// #include <QTime>
#include <QSharedPointer>
#include "DataThread.h"

class Observer : public QQuickItem {
    Q_OBJECT

public:
    // QTime _lastMousePress;
    // int   _clickThresholdMS = 300;

    Observer() {
        setFiltersChildMouseEvents(true);
        // connect(sender(), &QObject::destroyed, this, &Observer::hoveredTextPosition);
    }

    bool childMouseEventFilter(QQuickItem *, QEvent *event) override;
public slots:
    void hoveredTextPosition(int pos);
    void getDictionary(const ptrDictionary& zh_dict);
    void getDic(){ qDebug() << " Dic received ";}

signals:
    void hovered(int x, int y);
    void clicked();
    void textUpdate(QString newText);
    void doubleClicked();

private:
    QSharedPointer<ZH_Dictionary> zh_dict;
    std::string annotated;
};
