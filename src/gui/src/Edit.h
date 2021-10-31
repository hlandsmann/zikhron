#pragma once

#include <qobjectdefs.h>
#include <qquickitem.h>
#include <qstring.h>
#include "DataThread.h"
class QObject;

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
