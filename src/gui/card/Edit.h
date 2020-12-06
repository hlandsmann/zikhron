#pragma once

#include <QGuiApplication>
#include <QMouseEvent>
#include <QQuickItem>
#include <QSharedPointer>
#include "DataThread.h"

namespace card {
class Edit : public QQuickItem {
    Q_OBJECT
public:
    Edit();

public slots:
    void getDictionary(const PtrDictionary &zh_dict);
    void getCard(const PtrCard &ptrCard);
};

}  // namespace card
