#include <fmt/format.h>
#include <qqmlcontext.h>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <algorithm>
#include <iostream>

#include "Display.h"
#include "TextCard.h"
#include "ZH_Annotator.h"
#include "utils/Markup.h"

namespace card {
Display::Display() {
    setFiltersChildMouseEvents(true);
    // connect(sender(), &QObject::destroyed, this, &Display::hoveredTextPosition);
}

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

auto genPopupText(const ZH_Annotator::ZH_dicItemVec &items) -> std::string {
    return std::accumulate(
        items.begin(), items.end(), std::string{}, [](std::string &&a, const ZH_Dictionary::Item &b) {
            return a += fmt::format("<tr><td>{}</td><td>{}</td><td>{}</td></tr>",
                                    b.key,
                                    b.pronounciation,
                                    b.meanings.at(1));
        });
}

// "<div><table border='1'><caption><h4>Test stats</h4>"+
// "</caption><tr bgcolor='#9acd32'><th/><th>Number1</th><th>Number2</th></tr> <tr><th>Line1</th>"+
// "<td> 0 </td> <td> 1 </td> </tr> <tr><th>Line2</th> <td> 0 </td> <td> 1 </td> </tr>"+
// "<tr><th>Line3</th> <td> 0 </td> <td> 0 </td> </tr> <tr><th>Line4</th> <td> 1 </td> <td> 0 </td>
// </tr>"+
// "<tr><th>Line5</th> <td> 1 </td> <td> 1 </td> </tr> <tr><th>Line6</th> <td> 1 </td> <td> 1 </td> </tr>
// </div>"
void Display::clickedTextPosition(int pos) {
    if (!zh_annotator)
        return;

    const std::size_t index = paragraph.getWordIndex(pos);
    if (index >= zh_annotator->Items().size())
        return;

    const ZH_Annotator::Item &item = zh_annotator->Items().at(index);
    // const ZH_Annotator::Item &items = zh_annotator->Items().at(index);
    if (item.dicItemVec.empty())
        return;
    // const auto &dicitem = item.dicItem.value();
    // const std::string dic_entry = fmt::format(
    //     "{} ({}) - {}", dicitem.key, dicitem.pronounciation, dicitem.meanings.at(1));

    std::string table =
        std::string("") +
        "<table>"
        "<tr><th>Line1</th>" +
        "<td> 0 </td> <td> 1 </td> </tr> <tr><th>Line2</th> <td> 0 </td> <td> 1 </td> </tr>" +
        "<tr><th>Line3</th> <td> 0 </td> <td> 0 </td> </tr> <tr><th>Line4</th> <td> 1 </td> <td> 0 "
        "</td> </tr>" +
        "<tr><th>Line5</th> <td> 1 </td> <td> 1 </td> </tr> <tr><th>Line6</th> <td> 1 </td> <td> 1 "
        "</td> </tr> </table>";
    std::string popupText = genPopupText(item.dicItemVec);
    emit openPopup(paragraph.getWordStartPosition(pos), QString::fromStdString(popupText));
}

void Display::useCard() {
    if (ptrCard == nullptr || zh_dict == nullptr)
        return;
    auto maxText = ptrCard->getTextVector().front();

    zh_annotator = std::make_unique<ZH_Annotator>(maxText, zh_dict);
    std::transform(zh_annotator->Items().begin(),
                   zh_annotator->Items().end(),
                   std::back_inserter(paragraph),
                   [](const ZH_Annotator::Item &item) -> markup::Word {
                       if (not item.dicItemVec.empty())
                           return {.word = item.text, .color = 0, .backGroundColor = 0x010101};
                       return item.text;
                   });

    emit textUpdate(QString::fromStdString(paragraph.get()));
}

void Display::getDictionary(const PtrDictionary &_zh_dict) {
    zh_dict = _zh_dict.get();
    useCard();
}

void Display::getCard(const PtrCard &_ptrCard) {
    ptrCard = _ptrCard.get();
    useCard();
}

}  // namespace card
