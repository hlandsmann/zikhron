#include <qqmlcontext.h>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <iostream>

#include <unicode/unistr.h>
#include "Observer.h"
#include "TextCard.h"
#include "ZH_Annotator.h"

namespace {


void ReplaceStringInPlace(std::string& subject, const std::string& search,
                          const std::string& replace) {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
         subject.replace(pos, search.length(), replace);
         pos += replace.length();
    }
}

}  // namespace

bool Observer::childMouseEventFilter(QQuickItem *, QEvent *event) {
    // if false this will allow the event to continue as normal
    // if true it will stop the event propagating
    bool handled = false;
    // Q_PROPERTY(

    // https://doc.qt.io/qt-5/qevent.html#Type-enum
    QEvent::Type t = event->type();
    switch (t) {
    case QEvent::HoverMove: {
        QMouseEvent *e = static_cast<QMouseEvent *>(event);
        // qDebug() << "hover";
        emit hovered(e->x(), e->y());
        break;
    }
    // case QEvent::TouchUpdate: qDebug() << "touch event"; break;
    case QEvent::KeyPress:
    case QEvent::KeyRelease: {

        // QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        // qDebug("key press %d", keyEvent->key());
    } break;
    case QEvent::MouseButtonPress: {
        ReplaceStringInPlace(annotated, "#fff", "#e0e");
        emit textUpdate(QString::fromStdString(annotated));
        qDebug() << annotated.c_str() << "\n";
        // qDebug() << "mouse press";
    } break;
    case QEvent::MouseButtonRelease: {
        ReplaceStringInPlace(annotated, "#e0e", "#fff");
        emit textUpdate(QString::fromStdString(annotated));
        qDebug() << annotated.c_str() << "\n";
    }break;

    case QEvent::MouseButtonDblClick: {
        // QMouseEvent* mevent = static_cast<QMouseEvent*>(event);
        // qDebug() << "mouse double click";
        emit doubleClicked();
        // emit textUpdate("<strong>This text is important!</strong>");
        handled = true;
    } break;
    default: break;
    }

    return handled;
}

void Observer::hoveredTextPosition(int/* pos*/) {/*qDebug("signal pos  %d", pos);*/}
void Observer::getDictionary(const ptrDictionary &_zh_dict) {
    // qDebug()<< "Dictionary size: " << zh_dict.get()->Simplified().size();
    zh_dict = _zh_dict.get();

    CardDB cardDB;
    try {
        // dic.loadFromJson("/home/harmen/src/zikhron/cedict_u8.json");
        cardDB.loadFromSingleJson("/home/harmen/src/zikhron/cards/cards_u8.json");
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
    } catch (...) {
        std::cout << "Unknown Error" << std::endl;
    }

    icu::UnicodeString maxText = "";
    for (const auto &card : cardDB.cards)
        for (const auto &text : card->getTextVector()) {
            if (text.length() > maxText.length())
                maxText = text;
            // if (std::abs(30 - text.length()) < std::abs(30 - maxText.length()))
            //     maxText = text;
            // decode(dic, text);
        }
    try {
        // auto zh_dict = std::make_shared<ZH_Dictionary>("../dictionaries/handedict.u8");
        // auto zh_dict = std::make_shared<ZH_Dictionary>("../cedict_ts.u8");
        ZH_Annotator zh_annotater(maxText, zh_dict);
        annotated = zh_annotater.Annotated();
        emit textUpdate(QString::fromStdString(annotated));
    } catch (const std::exception &e) {
        std::cout << e.what() << "\n";
    }
    // emit textUpdate(QString::fromStdString(makeString(maxText)));
}
