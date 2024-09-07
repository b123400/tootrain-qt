#include "mainwindow.h"

#include <QApplication>
#include <QLabel>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include <QScreen>
#include <QTranslator>

#include "mastodonoauthwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTranslator translator;

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

    MainWindow w;

    w.show();

    return a.exec();
}
