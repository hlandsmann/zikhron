#pragma once

#include <QSharedPointer>
#include <QThread>
#include "ZH_Dictionary.h"

class ptrDictionary : public QObject {
    Q_OBJECT
public:
    ptrDictionary(QObject* parent = nullptr) { Q_UNUSED(parent); };
    ptrDictionary(const QSharedPointer<ZH_Dictionary>& qsptr) : _qsptr(qsptr){};
    ptrDictionary(const ptrDictionary& other) : QObject(), _qsptr(other.get()){};
    QSharedPointer<ZH_Dictionary> get() const { return _qsptr; }

private:
    QSharedPointer<ZH_Dictionary> _qsptr;
};

class DataThread : public QThread {
    Q_OBJECT
public:
    DataThread(QObject* parent = nullptr) { Q_UNUSED(parent); };
    // ~DataThread();
signals:
    void sendDictionary(const ptrDictionary& zh_dict);

protected:
    void run() override;

private:
};

Q_DECLARE_METATYPE(ptrDictionary)
