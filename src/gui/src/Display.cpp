#include "Display.h"
#include <QtCore/qglobal.h>
#include <annotation/Markup.h>
#include <annotation/ZH_Annotator.h>
#include <dictionary/ZH_Dictionary.h>
#include <fmt/format.h>
#include <qcoreevent.h>
#include <qdebug.h>
#include <qevent.h>
#include <spdlog/spdlog.h>
#include <utils/StringU8.h>
#include <algorithm>
#include <concepts>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <numeric>
#include <vector>
#include "DataThread.h"

namespace ranges = std::ranges;
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
    //     static int i=0;
    //     i++;
    // qDebug() << "Hello: " << i << "\n";
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
        // handled = true;
    } break;
    default: break;
    }

    return handled;
}

void Display::hoveredTextPosition(int pos) {
    // qDebug() << "Pos: " << pos << "\n";
    if (lastPos == pos)
        return;
    paragraph->undoChange();
    paragraph->highlightWordAtPosition(pos);
    emit textUpdate(QString::fromStdString(paragraph->get()));
    lastPos = pos;
}

auto genPopupText(const ZH_Annotator::ZH_dicItemVec &items) -> std::string {
    return std::accumulate(
        items.begin(), items.end(), std::string{}, [](std::string &&a, const ZH_Dictionary::Item &b) {
            return a += fmt::format(  // clang-format off
                       "<tr>"
                         "<td style=\"padding:0 15px 0 15px;\">{}</td>"
                         "<td style=\"padding:0 15px 0 15px;\">{}</td>"
                         "<td>{}</td>"
                       "</tr>",  // clang-format on
                       b.key,
                       b.pronounciation,
                       b.meanings.at(0));
        });
}

auto genPopupPosList(const ZH_Annotator::ZH_dicItemVec &items) -> QList<int> {
    QList<int> result;
    std::transform(items.begin(),
                   items.end(),
                   std::back_inserter(result),
                   [n = 1](const ZH_Dictionary::Item &item) mutable {
                       size_t temp = n;
                       n += static_cast<int>(utl::StringU8(item.key).length() + 1 +
                                             utl::StringU8(item.pronounciation).length() + 1 +
                                             utl::StringU8(item.meanings.at(0)).length() + 1);
                       return temp;
                   });
    return result;
}

// "<div><table border='1'><caption><h4>Test stats</h4>"+
// "</caption><tr bgcolor='#9acd32'><th/><th>Number1</th><th>Number2</th></tr> <tr><th>Line1</th>"+
// "<td> 0 </td> <td> 1 </td> </tr> <tr><th>Line2</th> <td> 0 </td> <td> 1 </td> </tr>"+
// "<tr><th>Line3</th> <td> 0 </td> <td> 0 </td> </tr> <tr><th>Line4</th> <td> 1 </td> <td> 0 </td>
// </tr>"+
// "<tr><th>Line5</th> <td> 1 </td> <td> 1 </td> </tr> <tr><th>Line6</th> <td> 1 </td> <td> 1 </td> </tr>
// </div>"
void Display::clickedTextPosition(int pos) {
    std::cout << "click pos: " << pos << "\n";
    const auto clickedItem = paragraph->wordFromPosition(pos);
    if (clickedItem.empty())
        return;
    const auto &word = clickedItem.front();
    std::cout << word.key << " : "
              << " : " << word.pronounciation << " : " << word.meanings.front() << "\n";

    if (!zh_annotator)
        return;
    spdlog::info("Hello World");

    const std::size_t index = paragraph->getWordIndex(pos);
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
        "<div><table border='1'><caption><h4>Test stats</h4>"
        "</caption><tr bgcolor='#9acd32'><th/><th>Number1</th><th>Number2</th></tr>"
        "<tr><th>Line1</th>" +
        "<td> 0 </td> <td> 1 </td> </tr> <tr><th>Line2</th> <td> 0 </td> <td> 1 </td> </tr>" +
        "<tr><th>Line3</th> <td> 0 </td> <td> 0 </td> </tr> <tr><th>Line4</th> <td> 1 </td> <td> 0 "
        "</td> </tr>" +
        "<tr><th>Line5</th> <td> 1 </td> <td> 1 </td> </tr> <tr><th>Line6</th> <td> 1 </td> <td> 1 "
        "</td> </tr> </table>";
    std::string table2 =
        "<table style=\"width: 100px;\">"
        "<tr>"
        " <td>"
        "<div style=\"word-wrap: break-word;\">"
        "test test test test test test test test"
        "</div>"
        " </td>"
        " <td><span style=\"display: inline;\">Short word</span></td>"
        " </tr>"
        "</table>";
    std::string popupText = genPopupText(item.dicItemVec);
    QList<int> popupPosList = genPopupPosList(item.dicItemVec);
    std::cout << popupText << "\n";
    std::cout << table2 << "\n";

    emit openPopup(
        paragraph->getWordStartPosition(pos), QString::fromStdString(popupText), popupPosList);
    // emit textUpdate(QString::fromStdString(popupText));
}

void Display::getParagraph(const PtrParagraph &_paragraph, const QList<int> &ease) {
    paragraph = _paragraph.get();
    // std::cout << "Pargarph got:  \n" << paragraph->get() << "\n";
    emit textUpdate(QString::fromStdString(paragraph->get()));
    // QList<int> vocPosList = {0, 15};
    QList<int> vocPosList;
    ranges::copy(paragraph->getVocablePositions(), std::back_inserter(vocPosList));

    emit vocableUpdate(QString::fromStdString(paragraph->getVocableString()), vocPosList, ease);
}

void Display::clickedEase(QList<int> ease) {
    // std::cout << "clicked Ease: " << ease << "\n";
    qDebug() << "clickedEase " << ease;
    emit sendEase(ease);

    // emit sendEase(mapIntToEase(0));
}

}  // namespace card