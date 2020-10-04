#include <unicode/unistr.h>
#include <exception>
#include <fstream>
#include <iostream>
#include <limits>
#include <numeric>
#include <span>
#include <stack>
#include <string>
#include "TextCard.h"
#include "ZH_Annotator.h"
#include "ZH_Dictionary.h"

#include <QArgument>
#include <QDate>
#include <QGuiApplication>
#include <QMetaObject>
#include <QQmlApplicationEngine>
#include <QQuickView>
#include <QUrl>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>
#include <QtGui>
#include <QtQuick>

#include <qqmlcontext.h>
#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <QSharedPointer>

#include "DataThread.h"
#include "gui/Observer.h"

// std::string makeString(const icu::UnicodeString& str) {
//     std::string tempString;
//     str.toUTF8String(tempString);
//     return tempString;
// }
int main(int argc, char* argv[]) {
    // CardDB cardDB;
    // try {
    //     // dic.loadFromJson("/home/harmen/src/zikhron/cedict_u8.json");
    //     cardDB.loadFromSingleJson("/home/harmen/src/zikhron/cards/cards_u8.json");
    // } catch (const std::exception& e) {
    //     std::cout << e.what() << std::endl;
    // } catch (...) {
    //     std::cout << "Unknown Error" << std::endl;
    // }

    // icu::UnicodeString maxText = "";
    // for (const auto& card : cardDB.cards)
    //     for (const auto& text : card->getTextVector()) {
    //         if (text.length() > maxText.length())
    //             maxText = text;
    //         // if (std::abs(30 - text.length()) < std::abs(30 - maxText.length()))
    //         //     maxText = text;
    //         // decode(dic, text);
    //     }

    // try {
    //     auto zh_dict = std::make_shared<ZH_Dictionary>("../dictionaries/handedict.u8");
    //     // auto zh_dict = std::make_shared<ZH_Dictionary>("../cedict_ts.u8");
    //     ZH_Annotator zh_annotater(makeString(maxText), zh_dict);
    // } catch (const std::exception& e) {
    //     std::cout << e.what() << "\n";
    // }
    try {
        QGuiApplication app(argc, argv);
        DataThread dataThread;

        // auto zh_dict = QSharedPointer<ZH_Dictionary>::create("../dictionaries/handedict.u8");

        qmlRegisterType<Observer>("MyObserver", 1, 0, "MyObserver");
        QQmlApplicationEngine engine;
        engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

        qRegisterMetaType<ptrDictionary>();
        auto myObserver = engine.rootObjects().first()->findChild<Observer*>("MyObserver");
        QObject::connect(&dataThread, &DataThread::sendDictionary, myObserver, &Observer::getDictionary);

        dataThread.start();
        // QObject::connect(
        //     (QObject*)engine.rootContext(), SIGNAL(textChanged(QString)), this,
        //     SLOT(someSlot(QString)));
        return app.exec();
    } catch (const std::exception& e) {
        std::cout << "Eception occured: " << e.what() << "\n";
    } catch (...) { std::cout << "Unknown exception occured \n"; }
}

// using std::cout;

// std::string makeString(const icu::UnicodeString& str) {
//     std::string tempString;
//     str.toUTF8String(tempString);
//     return tempString;
// }

// int main(int argc, char* argv[]) {
//     QGuiApplication app(argc, argv);
//     QCoreApplication::addLibraryPath("./");
//     QQmlApplicationEngine engine;
//     engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

//     // C++
//     // QQuickView view(QUrl::fromLocalFile("qrc:/main.qml"));
//     QQuickView   view(QUrl(QStringLiteral("qrc:/test_item.qml")));
//     QVariantList list;
//     list << 10 << QColor(Qt::green) << "bottles";

//     QVariantMap map;
//     map.insert("language", "QML");
//     map.insert("released", QDate(2010, 9, 21));

//     QMetaObject::invokeMethod(view.rootObject(),
//                               "readValues",
//                               Q_ARG(QVariant, QVariant::fromValue(list)),
//                               Q_ARG(QVariant, QVariant::fromValue(map)));

// CardDB cardDB;
// try {
//     // dic.loadFromJson("/home/harmen/src/zikhron/cedict_u8.json");
//     cardDB.loadFromSingleJson("/home/harmen/src/zikhron/cards/cards_u8.json");
// } catch (const std::exception& e) {
//     std::cout << e.what() << std::endl;
// } catch (...) {
//     std::cout << "Unknown Error" << std::endl;
// }

// icu::UnicodeString maxText = "";
// for (const auto& card : cardDB.cards)
//     for (const auto& text : card->getTextVector()) {
//         if (text.length() > maxText.length())
//             maxText = text;
//         // if (std::abs(30 - text.length()) < std::abs(30 - maxText.length()))
//         //     maxText = text;
//         // decode(dic, text);
//     }

//     // try {
//     //     auto zh_dict = std::make_shared<ZH_Dictionary>("../dictionaries/handedict.u8");
//     //     // auto zh_dict = std::make_shared<ZH_Dictionary>("../cedict_ts.u8");
//     //     ZH_Annotator zh_annotater(makeString(maxText), zh_dict);
//     // } catch (const std::exception& e) {
//     //     std::cout << e.what() << "\n";
//     // }
//     return app.exec();
// }
