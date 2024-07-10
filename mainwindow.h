#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWebSocket>
#include <QQueue>

#include "settingwindow.h"
#include "status.h"
#include "animationstate.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void showStatus(Status *status);
    int yForNewStatus(QSize statusSize);

public slots:
    void preferencesTriggered(bool check);
    void onWebSocketTextMessageReceived(QString message);
    void onWebSocketConnected();
    void onWebSocketDisconnected();
    void onWebSocketErrorOccurred(QAbstractSocket::SocketError error);
    void onAnimationFinish();
    void onRepaintTimer();
    void onCurrentAccountChanged();
    void onCurrentScreenChanged();

private:
    SettingWindow *settingWindow = nullptr;

    QQueue<AnimationState*> animationStates;

    void moveToScreen();

    QWebSocket webSocket;
    void startStreaming();
    void stopStreaming();
};
#endif // MAINWINDOW_H
