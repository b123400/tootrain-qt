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
    void moveToScreen();

public slots:
    void preferencesTriggered(bool check);
    void aboutDialogClicked(bool check);
    void onWebSocketTextMessageReceived(QString message);
    void onWebSocketConnected();
    void onWebSocketDisconnected();
    void onWebSocketErrorOccurred(QAbstractSocket::SocketError error);
    void onAnimationFinish();
    void onRepaintTimer();
    void onPingTimer();
    void onCurrentAccountChanged();
    void onCurrentScreenChanged();
    void onStatusEmojisLoaded(Status *status);
    void onReconnectTimer();

private:
    SettingWindow *settingWindow = nullptr;

    QQueue<AnimationState*> animationStates;

    QWebSocket webSocket;
    void startStreaming();
    void stopStreaming();
    void reconnect(bool afterAWhile);
    bool scheduledReconnect;
};
#endif // MAINWINDOW_H
