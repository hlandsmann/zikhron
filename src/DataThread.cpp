#include "DataThread.h"
#include <QDebug>

void DataThread::run() {
    auto zh_dict = QSharedPointer<ZH_Dictionary>::create("../dictionaries/handedict.u8");
    qDebug() << "Created Dictionary";

    emit sendDictionary(zh_dict);
}
