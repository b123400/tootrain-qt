#include "mainwindow.h"
#include <QMenuBar>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QLayout>
#include <QThread>
#include <QCoreApplication>
#include <QTextDocument>
#include <QTimer>
#include <QtSystemDetection>
#include <QSystemTrayIcon>

#include "settingwindow.h"
#include "settingmanager.h"
#include "mastodon/streamevent.h"
#include "dummystatus.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
// TODO: sth like a tray icon for accessing preference window on windows
#ifdef Q_OS_APPLE
    QMenuBar *menubar = this->menuBar();
    QMenu *fileMenu = menubar->addMenu("File");
    QAction *preferenceAction = fileMenu->addAction("Preferences");
    preferenceAction->setMenuRole(QAction::PreferencesRole);
    connect(preferenceAction, &QAction::triggered, this, &MainWindow::preferencesTriggered);
#endif

    QSystemTrayIcon *trayIcon = new QSystemTrayIcon();
    QIcon icon = QIcon(":/images/icon_128.png");
    trayIcon->setIcon(icon);
    QMenu *trayMenu = new QMenu(this);
    trayIcon->setContextMenu(trayMenu);
    QAction *trayPreferenceAction = trayMenu->addAction("Preferences...");
    trayPreferenceAction->setMenuRole(QAction::NoRole);
    connect(trayPreferenceAction, &QAction::triggered, this, &MainWindow::preferencesTriggered);
    QAction *trayQuitAction = trayMenu->addAction("Quit");
    trayQuitAction->setMenuRole(QAction::NoRole);
    connect(trayQuitAction, &QAction::triggered, QCoreApplication::instance(), &QCoreApplication::quit);
    trayIcon->show();

    connect(&webSocket, &QWebSocket::connected, this, &MainWindow::onWebSocketConnected);
    connect(&webSocket, &QWebSocket::textMessageReceived, this, &MainWindow::onWebSocketTextMessageReceived);
    connect(&webSocket, &QWebSocket::errorOccurred, this, &MainWindow::onWebSocketErrorOccurred);
    connect(&webSocket, &QWebSocket::disconnected, this, &MainWindow::onWebSocketDisconnected);
    connect(&SettingManager::shared(), &SettingManager::currentAccountChanged, this, &MainWindow::onCurrentAccountChanged);

    startStreaming();

#ifdef Q_OS_WIN
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::onRepaintTimer);
    timer->start(17); // 60fps

    // TODO: Show preference window in system tray
    this->preferencesTriggered(false);
#endif
}

MainWindow::~MainWindow() {}

void MainWindow::preferencesTriggered(bool checked) {
    if (!this->settingWindow) {
        this->settingWindow = new SettingWindow();
    }
    this->settingWindow->show();
}

void MainWindow::onRepaintTimer() {
    this->repaint();
}

void MainWindow::startStreaming() {
    auto accounts = SettingManager::shared().getAccounts();
    if (accounts.size()) {
        // Only take the first one
        auto account = accounts.at(0);
        // for (int i = 1; i < accounts.size(); i++) {
        //     delete accounts.at(i);
        // }

        QNetworkRequest request = QNetworkRequest(account->getWebSocketUrl());
        webSocket.open(request);
    }
}

void MainWindow::stopStreaming() {
    if (!webSocket.isValid()) return;
    webSocket.close();
}

void MainWindow::onWebSocketTextMessageReceived(QString message) {
    // qDebug() << "message: " << message;
    QJsonDocument jsonDoc((QJsonDocument::fromJson(message.toUtf8())));
    QJsonObject jsonReply = jsonDoc.object();
    if (MastodonStreamEvent::isValid(jsonReply)) {
        MastodonStreamEvent *se = new MastodonStreamEvent(jsonReply, this);
        qDebug() << "status: " << se;
        showStatus(se->status);
        delete se;
    }
}

void MainWindow::onWebSocketConnected() {
    qDebug() << "connected";
    auto s = new DummyStatus("Stream connected", this);
    showStatus(s);
    delete s;
}

void MainWindow::onWebSocketDisconnected() {
    qDebug() << "disconnected";
    auto s = new DummyStatus("Stream disconnected", this);
    showStatus(s);
    delete s;
}

void MainWindow::onWebSocketErrorOccurred(QAbstractSocket::SocketError error){
    qDebug() << "error";
}

void MainWindow::onCurrentAccountChanged() {
    this->stopStreaming();
    this->startStreaming();
}

void MainWindow::showStatus(Status *status) {
    QLabel *label = new QLabel(this);

    // TODO: Instead of just removing the format, we may leave the URL as is
    // Or maybe just detect the URL ourselves
    QTextDocument document;
    document.setHtml(status->getText());
    QString plainText = document.toPlainText();
    plainText = plainText.replace("\n", " ");
    // TODO: set it in config
    if (plainText.length() > 50) {
        plainText = plainText.first(50);
    }

    label->setText(plainText);

    QFont font = label->font();
    font.setPixelSize(40);
    font.setWeight(QFont::Weight::Bold);
    label->setFont(font);

    label->adjustSize();
    int y = yForNewStatus(label->size());
    if (y < 0) {
        qDebug() << "Too much on screen, not showing this status";
        return;
    }

    label->setTextFormat(Qt::TextFormat::PlainText);

    QPalette sample_palette;
    sample_palette.setColor(QPalette::WindowText, Qt::white);

    // To debug with background colour:
    // sample_palette.setColor(QPalette::Window, Qt::black);
    // label->setAutoFillBackground(true);

    label->setPalette(sample_palette);

    label->show();
    this->show();

    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect;
    effect->setBlurRadius(20);
    effect->setXOffset(0);
    effect->setYOffset(1);
    effect->setColor(QColor(0, 0, 0, 255));

    label->setGraphicsEffect(effect);


    QPropertyAnimation *anim = new QPropertyAnimation(label, "pos", this);
    anim->setDuration(10000);
    anim->setStartValue(QPoint(this->geometry().width(), y));
    anim->setEndValue(QPoint(-label->width(), y));
    anim->start();

    connect(anim, &QAbstractAnimation::finished, this, &MainWindow::onAnimationFinish);

    AnimationState *as = new AnimationState(this);
    as->target = label;
    as->animation = anim;
    this->animationStates.enqueue(as);
}

void MainWindow::onAnimationFinish() {
    qDebug() << "anim end";
    int i = 0;
    while (i < animationStates.length()) {
        auto animationState = animationStates[i];
        auto animation = animationState->animation;
        auto label = animationState->target;
        if (animation->state() == QAbstractAnimation::State::Stopped) {
            qDebug() << "deleting";
            animationStates.remove(i);
            delete label;
            delete animation;
            delete animationState;
        } else {
            i++;
        }
    }
}

int MainWindow::yForNewStatus(QSize statusSize) {
    qDebug() << "New status size: " << statusSize;
    auto winWidth = this->geometry().width();
    auto winHeight = this->geometry().height();
    int durationForThisStatus = 10000 * winWidth / (winWidth + statusSize.width());
    int currentY = 0;

    int counter = 0; // Just in case
    while (counter < 10000) {
        bool updated = false;
        for (auto animationState : animationStates) {
            int thisTime = animationState->animation->currentTime();
            int duration = animationState->animation->duration();
            int durationLeft = duration - thisTime;
            int thisTop = animationState->target->geometry().top();
            int thisBottom = animationState->target->geometry().bottom();

            int currentBottom = currentY + statusSize.height();

            // Time overlapping
            if (durationLeft > durationForThisStatus) {
                qDebug() << "time overlapping";
                // Y overlapping
                if ((currentY >= thisTop && currentY <= thisBottom) || (currentBottom >= thisTop && currentBottom <= thisBottom)) {
                    qDebug() << "currentY = " << thisBottom + 1;
                    currentY = thisBottom + 1;
                    updated = true;
                }
            }
        }
        if (!updated) {
            return currentY;
        }
        if (currentY > currentY + statusSize.height() /* lower than the screen */) {
            return -1;
        }
        counter++;
    }
    return -1;
}
