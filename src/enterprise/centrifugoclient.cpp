#include "centrifugoclient.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkRequest>
#include <QRandomGenerator>

CentrifugoClient::CentrifugoClient(const QString &name, const QString &version, QObject *parent)
    : QObject{parent}
    , name(name)
    , version(version)
{
    reconnectTimer.setSingleShot(true);
    connect(&reconnectTimer, &QTimer::timeout, this, &CentrifugoClient::open);
    pingTimer.setSingleShot(true);
    connect(&pingTimer, &QTimer::timeout, this, [this]() {
        retryLater(0, "no ping");
    });
}

void CentrifugoClient::connectToServer(const QUrl &endpoint, const QMap<QByteArray, QByteArray> &headers) {
    disconnectFromServer();
    this->endpoint = endpoint;
    this->headers = headers;
    wanted = true;
    open();
}

void CentrifugoClient::disconnectFromServer() {
    wanted = false;
    attempts = 0;
    positions = QJsonObject();
    dropSocket();
}

void CentrifugoClient::open() {
    dropSocket();
    // A socket of its own for every attempt: QWebSocket goes on reporting the
    // close code of the connection before, and that code is what says whether
    // to come back.
    socket = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);
    connect(socket, &QWebSocket::connected, this, [this]() {
        QJsonObject command{{"name", name}, {"version", version}};
        if (!positions.isEmpty()) command["subs"] = positions;
        QJsonObject message{{"id", 1}, {"connect", command}};
        socket->sendTextMessage(QString::fromUtf8(QJsonDocument(message).toJson(QJsonDocument::Compact)));
    });
    connect(socket, &QWebSocket::textMessageReceived, this, &CentrifugoClient::onTextMessage);
    connect(socket, &QWebSocket::disconnected, this, [this]() {
        int code = socket->closeCode();
        QString reason = socket->closeReason();
        if (isTerminal(code)) {
            giveUp(code, reason);
        } else {
            retryLater(code, reason);
        }
    });
    connect(socket, &QWebSocket::errorOccurred, this, [this](QAbstractSocket::SocketError) {
        retryLater(0, socket->errorString());
    });

    QNetworkRequest request(endpoint);
    for (auto it = headers.constBegin(); it != headers.constEnd(); ++it) {
        request.setRawHeader(it.key(), it.value());
    }
    socket->open(request);
}

void CentrifugoClient::dropSocket() {
    reconnectTimer.stop();
    pingTimer.stop();
    if (!socket) return;
    // Its signals first, so that closing it is not heard as a disconnect.
    socket->disconnect(this);
    socket->abort();
    socket->deleteLater();
    socket = nullptr;
}

void CentrifugoClient::giveUp(int code, const QString &reason) {
    disconnectFromServer();
    emit disconnected(code, reason);
}

// Waits longer after each attempt that fails, up to twenty seconds, and never
// exactly as long as another client would: a server coming back up is not met
// by every app at once.
void CentrifugoClient::retryLater(int code, const QString &reason) {
    dropSocket();
    if (!wanted) return;
    int ceiling = qMin(20000, 500 << qMin(attempts, 6));
    attempts++;
    reconnectTimer.start(ceiling / 2 + QRandomGenerator::global()->bounded(ceiling / 2));
    emit connecting(code, reason);
}

// A frame can carry several replies, one to a line.
void CentrifugoClient::onTextMessage(const QString &message) {
    for (const QString &line : message.split('\n', Qt::SkipEmptyParts)) {
        // A reply can end the connection, and what follows it in the frame
        // goes with it.
        if (!socket) return;
        onReply(QJsonDocument::fromJson(line.toUtf8()).object());
    }
}

void CentrifugoClient::onReply(const QJsonObject &reply) {
    if (reply.isEmpty()) {
        // The server's ping.
        if (answersPings) socket->sendTextMessage("{}");
        if (pingTimer.interval() > 0) pingTimer.start();
        return;
    }
    if (reply.contains("error")) {
        // The connect command was refused. Only a refusal the server calls
        // temporary is worth another attempt.
        const QJsonValue error = reply["error"];
        if (error["temporary"].toBool()) {
            retryLater(error["code"].toInt(), error["message"].toString());
        } else {
            giveUp(error["code"].toInt(), error["message"].toString());
        }
        return;
    }
    if (reply.contains("connect")) {
        const QJsonValue result = reply["connect"];
        attempts = 0;
        answersPings = result["pong"].toBool();
        // Ten seconds' grace on top of the server's own schedule.
        pingTimer.setInterval(result["ping"].toInt() > 0 ? (result["ping"].toInt() + 10) * 1000 : 0);
        if (pingTimer.interval() > 0) pingTimer.start();
        emit connected(result["client"].toString());

        // The channels this connection has, which may be fewer than the last
        // one had.
        const QJsonObject subscriptions = result["subs"].toObject();
        for (const QString &channel : positions.keys()) {
            if (subscriptions.contains(channel)) continue;
            positions.remove(channel);
            emit unsubscribed(channel);
        }
        for (auto it = subscriptions.constBegin(); it != subscriptions.constEnd(); ++it) {
            const QJsonValue subscription = it.value();
            onSubscribed(it.key(), subscription);
            // What was published while the connection was down.
            for (const QJsonValue &missed : subscription["publications"].toArray()) {
                onPublication(it.key(), missed);
            }
        }
        return;
    }
    const QJsonValue push = reply["push"];
    QString channel = push["channel"].toString();
    if (push["pub"].isObject()) {
        onPublication(channel, push["pub"]);
    } else if (push["subscribe"].isObject()) {
        onSubscribed(channel, push["subscribe"]);
    } else if (push["unsubscribe"].isObject()) {
        positions.remove(channel);
        emit unsubscribed(channel);
    } else if (push["disconnect"].isObject()) {
        int code = push["disconnect"]["code"].toInt();
        QString reason = push["disconnect"]["reason"].toString();
        if (isTerminal(code)) {
            giveUp(code, reason);
        } else {
            retryLater(code, reason);
        }
    }
}

void CentrifugoClient::onSubscribed(const QString &channel, const QJsonValue &subscription) {
    if (subscription["recoverable"].toBool()) {
        positions[channel] = QJsonObject{
            {"recover", true},
            {"epoch", subscription["epoch"]},
            {"offset", subscription["offset"]},
        };
    } else {
        positions.remove(channel);
    }
    emit subscribed(channel);
}

void CentrifugoClient::onPublication(const QString &channel, const QJsonValue &pub) {
    if (positions.contains(channel) && !pub["offset"].isUndefined()) {
        QJsonObject position = positions[channel].toObject();
        position["offset"] = pub["offset"];
        positions[channel] = position;
    }
    emit publication(channel, pub["data"].toObject());
}
