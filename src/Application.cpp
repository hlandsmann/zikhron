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
#include "gui/card/Annotate.h"
#include "gui/card/Display.h"
#include "gui/card/Edit.h"

int main(int argc, char* argv[]) {
    try {
        QGuiApplication app(argc, argv);
        app.setOrganizationName("zikhron");
        // app.setOrganizationDomain("https://github.com/hlandsmann/zikhron");
        app.setApplicationName("zikhron");
        QSettings settings;
        qDebug() << settings.fileName();
        DataThread dataThread;

        qmlRegisterType<card::Annotate>("CardAnnotate", 1, 0, "CardAnnotate");
        qmlRegisterType<card::Display>("CardDisplay", 1, 0, "CardDisplay");
        qmlRegisterType<card::Edit>("CardEdit", 1, 0, "CardEdit");
        QQmlApplicationEngine engine;
        engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

        qRegisterMetaType<PtrDictionary>();
        qRegisterMetaType<PtrCard>();
        auto cardAnnotate = engine.rootObjects().first()->findChild<card::Annotate*>("CardAnnotate");
        auto cardDisplay = engine.rootObjects().first()->findChild<card::Display*>("CardDisplay");
        auto cardEdit = engine.rootObjects().first()->findChild<card::Edit*>("CardEdit");

        QObject::connect(
            &dataThread, &DataThread::sendDictionary, cardAnnotate, &card::Annotate::getDictionary);
        QObject::connect(
            &dataThread, &DataThread::sendDictionary, cardDisplay, &card::Display::getDictionary);
        QObject::connect(&dataThread, &DataThread::sendCard, cardAnnotate, &card::Annotate::getCard);
        QObject::connect(&dataThread, &DataThread::sendCard, cardDisplay, &card::Display::getCard);
        QObject::connect(&dataThread, &DataThread::sendCard, cardEdit, &card::Edit::getCard);

        dataThread.start();
        // QObject::connect(
        //     (QObject*)engine.rootContext(), SIGNAL(textChanged(QString)), this,
        //     SLOT(someSlot(QString)));
        return app.exec();
    } catch (const std::exception& e) {
        std::cout << "Eception occured: " << e.what() << "\n";
    } catch (...) { std::cout << "Unknown exception occured \n"; }
}
