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
    QScreen *screen = QGuiApplication::primaryScreen();
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
    // QLabel *label = new QLabel(&w);
    // label->setText("Hello world");
    // QPalette sample_palette;
    // // sample_palette.setColor(QPalette::Window, Qt::white);
    // sample_palette.setColor(QPalette::WindowText, Qt::red);
    // // label->setAutoFillBackground(true);
    // label->setPalette(sample_palette);

    // QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect;
    // effect->setBlurRadius(5);
    // effect->setXOffset(5);
    // effect->setYOffset(5);
    // effect->setColor(Qt::black);

    // label->setGraphicsEffect(effect);
    auto geometry = screen->availableGeometry();
    w.setGeometry(geometry);
    // w.resize(geometry .height(), geometry .height());
    w.show();

    // QPropertyAnimation *anim = new QPropertyAnimation(label, "pos", &w);
    // anim->setDuration(10000);
    // anim->setStartValue(QPoint(600, 0));
    // anim->setEndValue(QPoint(0, 0));
    // anim->start();

    // MastodonOauthWindow oauthWindow;
    // oauthWindow.show();

    return a.exec();
}
