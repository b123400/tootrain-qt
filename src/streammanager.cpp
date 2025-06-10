#include <QTimer>
#include "streammanager.h"
#include "settingmanager.h"
#include "statusimageloader.h"
#include "dummystatus.h"

StreamManager::StreamManager(QObject *parent)
    : QObject{parent}
{
    connect(&StatusImageLoader::shared(), &StatusImageLoader::statusIsReady, this, &StreamManager::onStatusEmojisLoaded);

    QTimer *pingTimer = new QTimer(this);
    connect(pingTimer, &QTimer::timeout, this, &StreamManager::onPingTimer);
    pingTimer->start(60000);
}

void StreamManager::startStreaming(Account *account) {
    if (webSockets.contains(account->uuid)) return;
    QNetworkRequest request = QNetworkRequest(account->getWebSocketUrl());
    QWebSocket *webSocket = new QWebSocket();
    webSocket->open(request);

    connect(webSocket, &QWebSocket::connected, this, &StreamManager::onWebSocketConnected);
    connect(webSocket, &QWebSocket::textMessageReceived, this, &StreamManager::onWebSocketTextMessageReceived);
    connect(webSocket, &QWebSocket::errorOccurred, this, &StreamManager::onWebSocketErrorOccurred);
    connect(webSocket, &QWebSocket::disconnected, this, &StreamManager::onWebSocketDisconnected);

    webSockets.insert(account->uuid, webSocket);
}

void StreamManager::stopStreaming(Account *account) {
    if (!webSockets.contains(account->uuid)) return;
    QWebSocket *webSocket = webSockets.take(account->uuid);
    disconnect(webSocket, &QWebSocket::connected, this, &StreamManager::onWebSocketConnected);
    disconnect(webSocket, &QWebSocket::textMessageReceived, this, &StreamManager::onWebSocketTextMessageReceived);
    disconnect(webSocket, &QWebSocket::errorOccurred, this, &StreamManager::onWebSocketErrorOccurred);
    disconnect(webSocket, &QWebSocket::disconnected, this, &StreamManager::onWebSocketDisconnected);
    webSocket->close();
    delete webSocket;
}

void StreamManager::reconnect(Account *account) {
    stopStreaming(account);
    startStreaming(account);
}

bool StreamManager::isAccountStreaming(Account *account) {
    return webSockets.contains(account->uuid);
}

QList<QString> StreamManager::streamingAccountUuids() {
    return webSockets.keys();
}

void StreamManager::onWebSocketTextMessageReceived(QString message) {
    if (qobject_cast<QWebSocket*>(sender()) == nullptr) {
        return;
    }
    QWebSocket *webSocket = (QWebSocket*)sender();
    QString accountUuid = webSockets.key(webSocket);
    auto account = SettingManager::shared().accountWithUuid(accountUuid);
    if (account == nullptr) return;
    StreamEvent *se = account->getStreamEventFromWebSocketMessage(message);
    if (se == nullptr) return;
    Status *status = se->getStatus();
    if (status == nullptr) return;
    status->setParent(this);
    StatusImageLoader::shared().loadEmojisForStatus(status);
    delete se;
    delete account;
}

void StreamManager::onWebSocketConnected() {
    qDebug() << "connected";

    if (qobject_cast<QWebSocket*>(sender()) == nullptr) {
        return;
    }
    QWebSocket *webSocket = (QWebSocket*)sender();
    QString accountUuid = webSockets.key(webSocket);
    auto account = SettingManager::shared().accountWithUuid(accountUuid);

    auto s = new DummyStatus(tr("Stream connected: ") + account->fullUsername() , this);
    emit gotStatus(s);
    // TODO: delete s;

    if (account != nullptr) {
        account->connectedToWebSocket(webSocket);
        delete account;
    }
}

void StreamManager::onWebSocketDisconnected() {
    if (qobject_cast<QWebSocket*>(sender()) == nullptr) {
        return;
    }
    QWebSocket *webSocket = (QWebSocket*)sender();
    QString accountUuid = webSockets.key(webSocket);
    webSockets.remove(accountUuid);
    auto account = SettingManager::shared().accountWithUuid(accountUuid);

    connect(webSocket, &QWebSocket::connected, this, &StreamManager::onWebSocketConnected);
    connect(webSocket, &QWebSocket::textMessageReceived, this, &StreamManager::onWebSocketTextMessageReceived);
    connect(webSocket, &QWebSocket::errorOccurred, this, &StreamManager::onWebSocketErrorOccurred);
    connect(webSocket, &QWebSocket::disconnected, this, &StreamManager::onWebSocketDisconnected);

    auto closeCode = webSocket->closeCode();
    delete webSocket;

    auto s = new DummyStatus(tr("Stream disconnected"), this);
    emit gotStatus(s);
    // TODO: delete s;

    if (closeCode != QWebSocketProtocol::CloseCodeNormal) {
        if (account != nullptr) {
            reconnect(account);
            delete account;
        }
    }
}

QString printSocketError(QAbstractSocket::SocketError error) {
    switch (error) {
    case QAbstractSocket::SocketError::ConnectionRefusedError:
        return "ConnectionRefusedError";
    case QAbstractSocket::SocketError::RemoteHostClosedError:
        return "RemoteHostClosedError";
    case QAbstractSocket::SocketError::HostNotFoundError:
        return "HostNotFoundError";
    case QAbstractSocket::SocketError::SocketAccessError:
        return "SocketAccessError";
    case QAbstractSocket::SocketError::SocketResourceError:
        return "SocketResourceError";
    case QAbstractSocket::SocketError::SocketTimeoutError:
        return "SocketTimeoutError";
    case QAbstractSocket::SocketError::DatagramTooLargeError:
        return "DatagramTooLargeError";
    case QAbstractSocket::SocketError::NetworkError:
        return "NetworkError";
    case QAbstractSocket::SocketError::AddressInUseError:
        return "AddressInUseError";
    case QAbstractSocket::SocketError::SocketAddressNotAvailableError:
        return "SocketAddressNotAvailableError";
    case QAbstractSocket::SocketError::UnsupportedSocketOperationError:
        return "UnsupportedSocketOperationError";
    case QAbstractSocket::SocketError::UnfinishedSocketOperationError:
        return "UnfinishedSocketOperationError";
    case QAbstractSocket::SocketError::ProxyAuthenticationRequiredError:
        return "ProxyAuthenticationRequiredError";
    case QAbstractSocket::SocketError::SslHandshakeFailedError:
        return "SslHandshakeFailedError";
    case QAbstractSocket::SocketError::ProxyConnectionRefusedError:
        return "ProxyConnectionRefusedError";
    case QAbstractSocket::SocketError::ProxyConnectionClosedError:
        return "ProxyConnectionClosedError";
    case QAbstractSocket::SocketError::ProxyConnectionTimeoutError:
        return "ProxyConnectionTimeoutError";
    case QAbstractSocket::SocketError::ProxyNotFoundError:
        return "ProxyNotFoundError";
    case QAbstractSocket::SocketError::ProxyProtocolError:
        return "ProxyProtocolError";
    case QAbstractSocket::SocketError::OperationError:
        return "OperationError";
    case QAbstractSocket::SocketError::SslInternalError:
        return "SslInternalError";
    case QAbstractSocket::SocketError::SslInvalidUserDataError:
        return "SslInvalidUserDataError";
    case QAbstractSocket::SocketError::TemporaryError:
        return "TemporaryError";
    case QAbstractSocket::SocketError::UnknownSocketError:
        return "UnknownSocketError";
    }
    return "";
}

void StreamManager::onWebSocketErrorOccurred(QAbstractSocket::SocketError error) {
    QString errorMessage = printSocketError(error);
    qDebug() << "error" << errorMessage;

    if (qobject_cast<QWebSocket*>(sender()) == nullptr) {
        return;
    }
    QWebSocket *webSocket = (QWebSocket*)sender();
    QString accountUuid = webSockets.key(webSocket);
    webSockets.remove(accountUuid);
    auto account = SettingManager::shared().accountWithUuid(accountUuid);
    if (account != nullptr) {
        reconnect(account);
        delete account;
    }
}

void StreamManager::onStatusEmojisLoaded(Status *status) {
    emit gotStatus(status);
}

void StreamManager::onPingTimer() {
    for (auto i = this->webSockets.cbegin(), end = this->webSockets.cend(); i != end; ++i) {
        auto webSocket = i.value();
        if (webSocket->state() == QAbstractSocket::SocketState::ConnectedState ) {
            webSocket->ping();
        }
    }
}
