#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWebSocket>

#include "settingwindow.h"
#include "status.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void showStatus(Status *status);

public slots:
    void preferencesTriggered(bool check);
    void onWebSocketTextMessageReceived(QString message);
    void onWebSocketConnected();
    void onWebSocketErrorOccurred(QAbstractSocket::SocketError error);
    void onAnimationFinish();

private:
    SettingWindow *settingWindow = nullptr;

    QWebSocket webSocket;
    void startStreaming();
};
#endif // MAINWINDOW_H
