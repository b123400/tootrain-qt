#include "misskeyaccount.h"
#include <QUrlQuery>
#include <QJsonDocument>
#include "./misskeystreamevent.h"

MisskeyAccount::MisskeyAccount(QJsonObject responseObject, QUrl baseUrl, QObject *parent)
    : Account{parent}
{
    uuid = QUuid::createUuid().toString(QUuid::StringFormat::WithoutBraces);
    id = responseObject["id"].toString();
    displayName = responseObject["name"].toString();
    username = responseObject["username"].toString();
    avatarUrl = responseObject["avatarUrl"].toString();
    this->baseUrl = baseUrl;
    QList<MisskeyStreamSource *> sources;
    sources.append(new MisskeyStreamSource(MisskeyStreamSource::Channel::Home, this));
    this->sources = sources; // default
}

MisskeyAccount::MisskeyAccount(QSettings *settings, QObject *parent) : Account {settings, parent}
{
    accessToken = settings->value("accessToken").toString();
    baseUrl = settings->value("baseUrl").toUrl();

    QList<MisskeyStreamSource *> sources;

    int size = settings->beginReadArray("sources");
    for (int i = 0; i < size; ++i) {
        settings->setArrayIndex(i);
        auto s = new MisskeyStreamSource(settings, this);
        sources.append(s);
    }
    settings->endArray();
    if (sources.isEmpty()) {
        sources.append(new MisskeyStreamSource(MisskeyStreamSource::Channel::Home, this));
    }
    this->sources = sources; // default
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

    settings->remove("sources");
    settings->beginWriteArray("sources");
    for (qsizetype i = 0; i < this->sources.size(); i++) {
        settings->setArrayIndex(i);
        MisskeyStreamSource *s = this->sources[i];
        s->saveToSettings(settings);
    }
    settings->endArray();
}

StreamEvent* MisskeyAccount::getStreamEventFromWebSocketMessage(QString message) {
    qDebug() << "message: " << message;
    QJsonDocument jsonDoc((QJsonDocument::fromJson(message.toUtf8())));
    QJsonObject jsonReply = jsonDoc.object();
    MisskeyStreamEvent *se = new MisskeyStreamEvent(jsonReply, this->baseUrl, this);
    if (!se->isValid()) {
        delete se;
        return nullptr;
    }
    return se;
}
