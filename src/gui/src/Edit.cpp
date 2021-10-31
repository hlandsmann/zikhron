#include "Edit.h"
#include <QtCore/qglobal.h>  // for qDebug
#include <qdebug.h>          // for QDebug

namespace card {
Edit::Edit() { setFiltersChildMouseEvents(true); }

void Edit::getDictionary(const PtrDictionary &/*_zh_dict*/) {
    qDebug() << "Edit Dictionary\n";
}

void Edit::getCard(const PtrCard &/*_ptrCard*/) {
    qDebug() << "Edit Card\n";
}

}  // namespace card
