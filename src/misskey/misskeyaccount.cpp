#include "misskeyaccount.h"
#include <QUrlQuery>
#include <QJsonDocument>

MisskeyAccount::MisskeyAccount(QJsonObject responseObject, QUrl baseUrl, QObject *parent)
    : Account{parent}
{
    uuid = QUuid::createUuid().toString(QUuid::StringFormat::WithoutBraces);
    id = responseObject["id"].toString();
    displayName = responseObject["name"].toString();
    username = responseObject["username"].toString();
    avatarUrl = responseObject["avatarUrl"].toString();
    this->baseUrl = baseUrl;
}

MisskeyAccount::MisskeyAccount(QSettings *settings, QObject *parent) : Account {settings, parent}
{
    accessToken = settings->value("accessToken").toString();
    baseUrl = settings->value("baseUrl").toUrl();
}

QUrl MisskeyAccount::getWebSocketUrl() {
    QUrl url = QUrl(baseUrl);
    url.setScheme("wss");
    url.setPath("/streaming");
    QUrlQuery query;
    query.addQueryItem("i", accessToken);
    url.setQuery(query);
    qDebug() << "stream url: " << url;
    return url;
}

void MisskeyAccount::connectedToWebSocket(QWebSocket *websocket) {
    QJsonObject params;
    QJsonObject body;
    body["channel"] = "localTimeline";
    body["id"] = QUuid::createUuid().toString(QUuid::StringFormat::WithoutBraces);
    body["params"] = params;
    QJsonObject obj;
    obj["type"] = "connect";
    obj["body"] = body;
    QJsonDocument doc = QJsonDocument(obj);
    auto message = doc.toJson();
    websocket->sendTextMessage(message);
}

QString MisskeyAccount::fullUsername() {
    if (this->baseUrl.isEmpty()) {
        return username;
    }
    return username + "@" + this->baseUrl.host();
}

void MisskeyAccount::saveToSettings(QSettings *settings) {
    this->Account::saveToSettings(settings);
    settings->setValue("type", "misskey");

    settings->setValue("baseUrl", this->baseUrl);
    settings->setValue("accessToken", this->accessToken);
}

MisskeyStreamEvent* MisskeyAccount::getStreamEventFromWebSocketMessage(QString message) {
    // TODO
    return nullptr;
}
