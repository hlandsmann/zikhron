#include <qqmlcontext.h>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <algorithm>
#include <iostream>

#include "HtmlGen.h"
#include "Observer.h"
#include "TextCard.h"
#include "ZH_Annotator.h"

namespace {

void ReplaceStringInPlace(std::string &subject, const std::string &search, const std::string &replace) {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
    }
}

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

void Observer::hoveredTextPosition(int /* pos*/) { /*qDebug("signal pos  %d", pos);*/
}

auto Observer::getLongText() const -> utl::StringU8 {
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

void Observer::getDictionary(const ptrDictionary &_zh_dict) {
    // qDebug()<< "Dictionary size: " << zh_dict.get()->Simplified().size();
    zh_dict = _zh_dict.get();
    auto maxText = getLongText();
    ZH_Annotator zh_annotater(maxText, zh_dict);
    markup::Paragraph paragraph;
    std::transform(zh_annotater.Items().begin(),
                   zh_annotater.Items().end(),
                   std::back_inserter(paragraph),
                   [](const ZH_Annotator::Item &item) -> markup::Word {
                       if (not item.dicItem.has_value())
                           return {.word = item.text, .color = 0, .backGroundColor = 0x010101};
                       return item.text;
                   });

    // std::transform(

    emit textUpdate(QString::fromStdString(paragraph.Get()));
}
