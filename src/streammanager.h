#ifndef STREAMMANAGER_H
#define STREAMMANAGER_H

#include <QObject>
#include <QTimer>
#include "account.h"

class StreamManager : public QObject
{
    Q_OBJECT
public:
    static StreamManager &shared() {static StreamManager manager; return manager;}
    explicit StreamManager(QObject *parent = nullptr);

    void startStreaming(Account *account);
    void stopStreaming(Account *account);
    void reconnect(Account *account, bool afterAWhile);
    bool isAccountStreaming(Account *account);
    QList<QString> streamingAccountUuids();

public slots:
    void onWebSocketTextMessageReceived(QString message);
    void onWebSocketConnected();
    void onWebSocketDisconnected();
    void onWebSocketErrorOccurred(QAbstractSocket::SocketError error);
    void onStatusEmojisLoaded(Status *status);
    void onPingTimer();
    void onReconnectTimer();

signals:
    void gotStatus(Status *status);

private:
    QMap<QString, QWebSocket *> webSockets;
    QSet<QString> rescheduledAccountUuid;
    QMap<QString, QTimer *> reconnectTimers;

    QList<QWebSocket *> unusedWebSockets;
};

#endif // STREAMMANAGER_H
