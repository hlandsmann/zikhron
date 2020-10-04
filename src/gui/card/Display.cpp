#include <qqmlcontext.h>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <algorithm>
#include <iostream>

#include "Display.h"
#include "TextCard.h"
#include "ZH_Annotator.h"
#include "gui/HtmlGen.h"

namespace {

auto loadCardDB() -> CardDB {
    CardDB cardDB;
    try {
        // dic.loadFromJson("/home/harmen/src/zikhron/cedict_u8.json");
        cardDB.loadFromSingleJson("/home/harmen/src/zikhron/cards/cards_u8.json");
    } catch (const std::exception &e) { std::cout << e.what() << std::endl; } catch (...) {
        std::cout << "Unknown Error" << std::endl;
    }
    return cardDB;
}

}  // namespace

namespace card {
bool Display::childMouseEventFilter(QQuickItem *, QEvent *event) {
    // if false this will allow the event to continue as normal
    // if true it will stop the event propagating
    bool handled = false;
    // Q_PROPERTY(

    // https://doc.qt.io/qt-5/qevent.html#Type-enum
    QEvent::Type t = event->type();
    switch (t) {
    case QEvent::HoverLeave: hoveredTextPosition(-1); break;
    case QEvent::HoverMove: {
        const QMouseEvent &e = static_cast<QMouseEvent &>(*event);
        if (e.x() >= 0 && e.y() >= 5)
            emit hovered(e.x(), e.y());
        else
            hoveredTextPosition(-1);
    } break;
    case QEvent::MouseButtonPress: {
        const QMouseEvent &e = static_cast<QMouseEvent &>(*event);
        emit clicked(e.x(), e.y());
    } break;
    // case QEvent::TouchUpdate: qDebug() << "touch event"; break;
    case QEvent::KeyPress:
    case QEvent::KeyRelease: {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        qDebug("key press %d", keyEvent->key());
    } break;

    case QEvent::MouseButtonRelease: {
        // qDebug() << "mouse release";

    } break;

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

void Display::hoveredTextPosition(int pos) {
    if (lastPos == pos)
        return;
    paragraph.undoChange();
    paragraph.changeWordAtPosition(pos, [](markup::Word &word) { word.setBackgroundColor(0x227722); });
    emit textUpdate(QString::fromStdString(paragraph.get()));
    lastPos = pos;
}

auto Display::getLongText() const -> utl::StringU8 {
    if (cardDB.cards.empty())
        cardDB = loadCardDB();

    icu::UnicodeString maxText = "";
    for (const auto &card : cardDB.cards)
        for (const auto &text : card->getTextVector()) {
            if (text.length() > maxText.length())
                maxText = text;
            // if (std::abs(30 - text.length()) < std::abs(30 - maxText.length()))
            //     maxText = text;
            // decode(dic, text);
        }
    return maxText;
}

void Display::getDictionary(const ptrDictionary &_zh_dict) {
    // qDebug()<< "Dictionary size: " << zh_dict.get()->Simplified().size();
    zh_dict = _zh_dict.get();
    auto maxText = getLongText();
    ZH_Annotator zh_annotater(maxText, zh_dict);
    std::transform(zh_annotater.Items().begin(),
                   zh_annotater.Items().end(),
                   std::back_inserter(paragraph),
                   [](const ZH_Annotator::Item &item) -> markup::Word {
                       if (not item.dicItem.has_value())
                           return {.word = item.text, .color = 0, .backGroundColor = 0x010101};
                       return item.text;
                   });

    emit textUpdate(QString::fromStdString(paragraph.get()));
}

}  // namespace card
