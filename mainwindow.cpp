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
#include <QScreen>
#include <QGuiApplication>

#include "settingwindow.h"
#include "settingmanager.h"
#include "mastodon/streamevent.h"
#include "dummystatus.h"
#include "statusimageloader.h"
#include <QApplication>

#include <QMovie>
#include <QTextBlock>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
#ifdef Q_OS_APPLE
    QMenuBar *menubar = this->menuBar();
    QMenu *fileMenu = menubar->addMenu(tr("File"));
    QAction *preferenceAction = fileMenu->addAction(tr("Preferences"));
    preferenceAction->setMenuRole(QAction::PreferencesRole);
    connect(preferenceAction, &QAction::triggered, this, &MainWindow::preferencesTriggered);
#endif

    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_OpaquePaintEvent, false);
    setAttribute(Qt::WA_TranslucentBackground, true);

    // setWindowOpacity(0.5);

    setWindowFlags(this->windowFlags()
                | Qt::WindowDoesNotAcceptFocus
                | Qt::WindowTransparentForInput
                | Qt::X11BypassWindowManagerHint
                | Qt::WindowStaysOnTopHint
                | Qt::NoDropShadowWindowHint
                | Qt::FramelessWindowHint
                );

    QSystemTrayIcon *trayIcon = new QSystemTrayIcon();
    QIcon icon = QIcon(":/images/16x16.png");
    trayIcon->setIcon(icon);
    QMenu *trayMenu = new QMenu(this);
    trayIcon->setContextMenu(trayMenu);
    QAction *trayPreferenceAction = trayMenu->addAction(tr("Preferences..."));
    trayPreferenceAction->setMenuRole(QAction::NoRole);
    connect(trayPreferenceAction, &QAction::triggered, this, &MainWindow::preferencesTriggered);
    QAction *trayQuitAction = trayMenu->addAction(tr("Quit"));
    trayQuitAction->setMenuRole(QAction::NoRole);
    connect(trayQuitAction, &QAction::triggered, QCoreApplication::instance(), &QCoreApplication::quit);
    trayIcon->show();

    connect(&StatusImageLoader::shared(), &StatusImageLoader::statusIsReady, this, &MainWindow::onStatusEmojisLoaded);

    connect(&webSocket, &QWebSocket::connected, this, &MainWindow::onWebSocketConnected);
    connect(&webSocket, &QWebSocket::textMessageReceived, this, &MainWindow::onWebSocketTextMessageReceived);
    connect(&webSocket, &QWebSocket::errorOccurred, this, &MainWindow::onWebSocketErrorOccurred);
    connect(&webSocket, &QWebSocket::disconnected, this, &MainWindow::onWebSocketDisconnected);

    QTimer *pingTimer = new QTimer(this);
    connect(pingTimer, &QTimer::timeout, this, &MainWindow::onPingTimer);
    pingTimer->start(60000);

    connect(&SettingManager::shared(), &SettingManager::currentAccountChanged, this, &MainWindow::onCurrentAccountChanged);

    connect(&SettingManager::shared(), &SettingManager::currentScreenChanged, this, &MainWindow::onCurrentScreenChanged);
    moveToScreen();

    startStreaming();

    auto accounts = SettingManager::shared().getAccounts();
    if (!accounts.size()) {
        this->preferencesTriggered(false);
    }
    for (auto a : accounts) {
        delete a;
    }

#ifdef Q_OS_WIN
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::onRepaintTimer);
    timer->start(17); // 60fps
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

void MainWindow::onPingTimer() {
    if (!webSocket.isValid()) return;
    webSocket.ping();
}

void MainWindow::moveToScreen() {
    QScreen *theScreen = SettingManager::shared().getScreen();
    this->setScreen(theScreen);
    auto geometry = theScreen->availableGeometry();
    setGeometry(geometry);
    show();
}

void MainWindow::startStreaming() {
    auto accounts = SettingManager::shared().getAccounts();
    if (accounts.size()) {
        // Only take the first one
        auto account = accounts.at(0);

        QNetworkRequest request = QNetworkRequest(account->getWebSocketUrl());
        webSocket.open(request);
        for (auto a : accounts) {
            delete a;
        }
    }
}

void MainWindow::stopStreaming() {
    if (!webSocket.isValid()) return;
    webSocket.close();
}

void MainWindow::onStatusEmojisLoaded(Status *status) {
    showStatus(status);
    // delete status;
}

void MainWindow::onWebSocketTextMessageReceived(QString message) {
    // qDebug() << "message: " << message;
    QJsonDocument jsonDoc((QJsonDocument::fromJson(message.toUtf8())));
    QJsonObject jsonReply = jsonDoc.object();
    if (MastodonStreamEvent::isValid(jsonReply)) {
        MastodonStreamEvent *se = new MastodonStreamEvent(jsonReply, this);
        qDebug() << "status: " << se;
        StatusImageLoader::shared().loadEmojisForStatus(se->status);
        se->status->setParent(this);
        delete se;
    }
}

void MainWindow::onWebSocketConnected() {
    qDebug() << "connected";
    auto s = new DummyStatus(tr("Stream connected"), this);
    showStatus(s);
    delete s;
}

void MainWindow::onWebSocketDisconnected() {
    qDebug() << "disconnected";
    auto s = new DummyStatus(tr("Stream disconnected"), this);
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

void MainWindow::onCurrentScreenChanged() {
    moveToScreen();
    auto s = new DummyStatus("This screen is selected", this);
    showStatus(s);
    delete s;
}

void MainWindow::showStatus(Status *status) {
    QWidget *baseWidget = new QWidget(this);

    QHBoxLayout *box = new QHBoxLayout(baseWidget);
    box->setContentsMargins(0,0,0,0);
    box->setSpacing(0);
    baseWidget->setLayout(box);

    QPalette palette;
    palette.setColor(QPalette::WindowText, Qt::white);
    // To debug with background colour:
    // palette.setColor(QPalette::Window, Qt::red);
    // label->setAutoFillBackground(true);

    int fontSize = 40;
    QFont font = QApplication::font();
    font.setPixelSize(fontSize);
    font.setWeight(QFont::Weight::Bold);

    qsizetype characterCount = 0;
    qsizetype characterCountLimit = 50;
    AnimationState *as = new AnimationState(this);

    auto components = status->richTextcomponents();
    for (auto component : components) {
        if (component->emoji) {
            QLabel *label = new QLabel(this);
            QMovie *mv = new QMovie(component->emoji->getPath());
            mv->setParent(as);
            mv->start();
            label->setScaledContents(true);
            label->setMovie(mv);
            label->setMargin(0);
            label->adjustSize();
            qDebug() << "label size: " << label->size();
            label->setFixedSize(label->width() * fontSize / label->height(), fontSize);
            label->show();
            box->addWidget(label);
        } else if (component->text.length()) {
            QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect(as);
            effect->setBlurRadius(20);
            effect->setXOffset(0);
            effect->setYOffset(1);
            effect->setColor(QColor(0, 0, 0, 255));

            qsizetype thisLength = component->text.length();
            QString text;
            if (characterCount + thisLength <= characterCountLimit) {
                text = component->text;
                characterCount += thisLength;
            } else {
                qsizetype charLeft = characterCountLimit - characterCount;
                text = component->text.sliced(0, charLeft);
                characterCount = characterCountLimit;
            }

            QLabel *label = new QLabel(this);
            label->setPalette(palette);
            label->setFont(font);
            label->setMargin(0);
            label->setText(text);
            label->setTextFormat(Qt::TextFormat::PlainText);
            label->adjustSize();
            label->show();
            box->addWidget(label);
            label->setGraphicsEffect(effect);

            if (characterCount >= characterCountLimit) {
                break;
            }
        }
    }

    baseWidget->adjustSize();

    int y = yForNewStatus(baseWidget->size());
    if (y < 0) {
        qDebug() << "Too much on screen, not showing this status";
        delete as;
        return;
    }

    baseWidget->show();
    this->show();

    QPropertyAnimation *anim = new QPropertyAnimation(baseWidget, "pos", this);
    anim->setDuration(10000);
    anim->setStartValue(QPoint(this->geometry().width(), y));
    anim->setEndValue(QPoint(-baseWidget->width(), y));
    anim->start();

    connect(anim, &QAbstractAnimation::finished, this, &MainWindow::onAnimationFinish);

    as->target = baseWidget;
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
