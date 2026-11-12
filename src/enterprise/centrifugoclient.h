#ifndef CENTRIFUGOCLIENT_H
#define CENTRIFUGOCLIENT_H

#include <QJsonObject>
#include <QMap>
#include <QObject>
#include <QTimer>
#include <QUrl>
#include <QWebSocket>

// A small Centrifugo client, in place of the centrifuge-swift library the Mac
// app links: the bidirectional WebSocket transport with the JSON protocol, and
// server-side subscriptions only, which is all the TootRain server uses. The
// app never subscribes to anything itself: the channels come with the
// connection.
//
// It reconnects on its own, and picks each channel up where it left off, until
// the server says not to come back.
class CentrifugoClient : public QObject
{
    Q_OBJECT
public:
    CentrifugoClient(const QString &name, const QString &version, QObject *parent = nullptr);

    // Opens the connection and keeps it open. `headers` go with the WebSocket
    // handshake, which is where the server's connect proxy reads who this is.
    void connectToServer(const QUrl &endpoint, const QMap<QByteArray, QByteArray> &headers);

    // Closes the connection for good. Nothing is signalled.
    void disconnectFromServer();

    // Centrifugo keeps the reconnect advice in the disconnect code itself:
    // 3500-3999 and 4500-4999 are the ranges that say "do not come back".
    static bool isTerminal(int code) { return (code >= 3500 && code < 4000) || (code >= 4500 && code < 5000); }

signals:
    void connected(QString clientId);
    // The connection is down and is being tried again.
    void connecting(int code, QString reason);
    // The server will not take this connection back, so the client has given
    // up for good: this is the last that is heard of it.
    void disconnected(int code, QString reason);
    void subscribed(QString channel);
    void unsubscribed(QString channel);
    void publication(QString channel, QJsonObject data);

private:
    void open();
    void dropSocket();
    void giveUp(int code, const QString &reason);
    void retryLater(int code, const QString &reason);
    void onTextMessage(const QString &message);
    void onReply(const QJsonObject &reply);
    void onSubscribed(const QString &channel, const QJsonValue &subscription);
    void onPublication(const QString &channel, const QJsonValue &pub);

    QString name;
    QString version;
    QUrl endpoint;
    QMap<QByteArray, QByteArray> headers;

    QWebSocket *socket = nullptr;
    // Whether a connection is wanted at all, which it is from
    // `connectToServer` until the server refuses for good or the app asks for
    // it to be closed.
    bool wanted = false;
    int attempts = 0;
    bool answersPings = false;
    QTimer reconnectTimer;
    // The server pings on a schedule it states when the connection opens, so
    // one that goes quiet for longer than that is a connection that has died.
    QTimer pingTimer;
    // Where each channel's stream was last read up to, as the `subs` of the
    // next connect command: what lets the server send what was missed while
    // the connection was down.
    QJsonObject positions;
};

#endif // CENTRIFUGOCLIENT_H
