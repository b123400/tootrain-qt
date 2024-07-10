#include "mainwindow.h"

#include <QApplication>
#include <QLabel>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include <QScreen>

#include "mastodonoauthwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    // TODO: support multiple screen
    w.setAttribute(Qt::WA_NoSystemBackground, true);
    w.setAttribute(Qt::WA_OpaquePaintEvent, false);
    w.setAttribute(Qt::WA_TranslucentBackground, true);

    // w.setWindowOpacity(0.5);

    w.setWindowFlags(w.windowFlags()
                     | Qt::WindowDoesNotAcceptFocus
                     | Qt::WindowTransparentForInput
                     | Qt::X11BypassWindowManagerHint
                     | Qt::WindowStaysOnTopHint
                     | Qt::NoDropShadowWindowHint
                     | Qt::FramelessWindowHint
                     );

    // w.resize(geometry .height(), geometry .height());
    w.show();

    return a.exec();
}
