#include <Annotate.h>               // for Annotate
#include <Display.h>                // for Display
#include <Edit.h>                   // for Edit
#include <QtCore/qglobal.h>         // for qDebug
#include <qdebug.h>                 // for QDebug
#include <qguiapplication.h>        // for QGuiApplication
#include <qlist.h>                  // for QList
#include <qmetatype.h>              // for qRegisterMetaType
#include <qobject.h>                // for QObject
#include <qqml.h>                   // for qmlRegisterType
#include <qqmlapplicationengine.h>  // for QQmlApplicationEngine
#include <qsettings.h>              // for QSettings
#include <qstringliteral.h>         // for QStaticStringData, QStringLiteral
#include <qurl.h>                   // for QUrl
#include <exception>                // for exception
#include <iostream>                 // for operator<<, basic_ostream, char_t...
#include <DataThread.h>             // for DataThread, PtrCard (ptr only)
_GLIBCXX_BEGIN_NAMESPACE_CONTAINER
_GLIBCXX_BEGIN_NAMESPACE_ALGO

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
        qRegisterMetaType<PtrParagraph>();
        auto cardAnnotate = engine.rootObjects().first()->findChild<card::Annotate*>("CardAnnotate");
        auto cardDisplay = engine.rootObjects().first()->findChild<card::Display*>("CardDisplay");
        auto cardEdit = engine.rootObjects().first()->findChild<card::Edit*>("CardEdit");

        QObject::connect(
            &dataThread, &DataThread::sendAnnotation, cardAnnotate, &card::Annotate::getAnnotation);

        QObject::connect(&dataThread, &DataThread::sendCard, cardEdit, &card::Edit::getCard);

        QObject::connect(
            &dataThread, &DataThread::sendParagraph, cardDisplay, &card::Display::getParagraph);
        QObject::connect(cardDisplay, &card::Display::sendEase, &dataThread, &DataThread::getCardEase);
        QObject::connect(cardAnnotate,
                         &card::Annotate::cardAnnotationChoice,
                         &dataThread,
                         &DataThread::cardAnnotationChoice);

        dataThread.start();
        // QObject::connect(
        //     (QObject*)engine.rootContext(), SIGNAL(textChanged(QString)), this,
        //     SLOT(someSlot(QString)));
        return app.exec();
    } catch (const std::exception& e) {
        std::cout << "Eception occured: " << e.what() << "\n";
    } catch (...) { std::cout << "Unknown exception occured \n"; }
}
