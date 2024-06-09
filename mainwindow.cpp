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

#include "settingwindow.h"
#include "settingmanager.h"
#include "mastodon/streamevent.h"
#include "dummystatus.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QMenuBar *menubar = this->menuBar();
    QMenu *fileMenu = menubar->addMenu("File");
    QAction *preferenceAction = fileMenu->addAction("Preferences");
    preferenceAction->setMenuRole(QAction::PreferencesRole);
    connect(preferenceAction, &QAction::triggered, this, &MainWindow::preferencesTriggered);

    connect(&webSocket, &QWebSocket::connected, this, &MainWindow::onWebSocketConnected);
    connect(&webSocket, &QWebSocket::textMessageReceived, this, &MainWindow::onWebSocketTextMessageReceived);
    connect(&webSocket, &QWebSocket::errorOccurred, this, &MainWindow::onWebSocketErrorOccurred);

    startStreaming();
}

MainWindow::~MainWindow() {}

void MainWindow::preferencesTriggered(bool checked) {
    if (!this->settingWindow) {
        this->settingWindow = new SettingWindow();
    }
    this->settingWindow->show();
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

void MainWindow::onWebSocketTextMessageReceived(QString message) {
    // qDebug() << "message: " << message;
    QJsonDocument jsonDoc((QJsonDocument::fromJson(message.toUtf8())));
    QJsonObject jsonReply = jsonDoc.object();
    if (MastodonStreamEvent::isValid(jsonReply)) {
        MastodonStreamEvent *se = new MastodonStreamEvent(jsonReply, this);
        qDebug() << "status: " << se;
        showStatus(se->status);
    }
}

void MainWindow::onWebSocketConnected() {
    qDebug() << "connected";
    auto s = new DummyStatus("Hello Fediverse", this);
    showStatus(s);
    delete s;
}

void MainWindow::onWebSocketErrorOccurred(QAbstractSocket::SocketError error){
    qDebug() << "error";
}

void MainWindow::showStatus(Status *status) {
    // const bool isGuiThread = QThread::currentThread() == QCoreApplication::instance()->thread();

    QLabel *label = new QLabel(this);
    // qDebug() << "message: " << status->getText();
    label->setText(status->getText());
    label->setFixedWidth(600);
    QPalette sample_palette;
    // sample_palette.setColor(QPalette::Window, Qt::white);
    sample_palette.setColor(QPalette::WindowText, Qt::red);
    // label->setAutoFillBackground(true);
    label->setPalette(sample_palette);

    label->show();
    this->show();

    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect;
    effect->setBlurRadius(5);
    effect->setXOffset(0);
    effect->setYOffset(1);
    effect->setColor(Qt::black);

    label->setGraphicsEffect(effect);

    QPropertyAnimation *anim = new QPropertyAnimation(label, "pos", this);
    anim->setDuration(10000);
    anim->setStartValue(QPoint(600, 0));
    anim->setEndValue(QPoint(0, 0));
    anim->start();

    connect(anim, &QAbstractAnimation::finished, this, &MainWindow::onAnimationFinish);
}

void MainWindow::onAnimationFinish() {
    qDebug() << "anim end";
    auto animations = this->findChildren<QPropertyAnimation*>();
    for (auto animation : animations) {
        auto target = animation->targetObject();
        if (animation->state() == QAbstractAnimation::State::Stopped && dynamic_cast<QLabel*>(target) != nullptr) {
            qDebug() << "deleting";
            QLabel *label = (QLabel*)target;
            delete label;
        }
    }
}
