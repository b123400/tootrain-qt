#include "mainwindow.h"

#include <QApplication>
#include <QLabel>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include <QScreen>
#include <QTranslator>
#include <QLocale>

#include "enterprise/tootrainurlscheme.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // What the TootRain server is told this app is.
    a.setApplicationVersion(TOOTRAIN_VERSION);
    // Started by a tootrain:// link, which belongs to the copy of the app that
    // is waiting for it.
    if (TootRainUrlScheme::handOverToRunningApp(a.arguments())) return 0;
    QTranslator translator;

    QLocale sysLocale = QLocale::system();
    QString localeName = sysLocale.name();
    bool isJa = localeName.startsWith("ja");

    if (isJa) {
        QString langPath;
    #if defined(Q_OS_WIN)
        langPath = "./tootrain-qt_ja.qm";
    #elif defined(Q_OS_MAC)
        langPath = "../Resources/tootrain-qt_ja.qm";
    #else
        langPath = "./tootrain-qt_ja.qm";
    #endif
        if (translator.load(QLocale(), langPath)) {
            qDebug() << "Installing translator";
            QCoreApplication::installTranslator(&translator);
        } else {
            qDebug() << "Lang not found!";
        }
    }

    MainWindow w;

    w.show();

    return a.exec();
}
