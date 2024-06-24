#include "account.h"
#include <QUuid>
#include <QUrlQuery>

MastodonAccount::MastodonAccount(QObject *parent)
    : Account{parent}
{}

MastodonAccount::MastodonAccount(QJsonObject json, QObject *parent) : Account {parent}
{
    uuid = QUuid::createUuid().toString(QUuid::StringFormat::WithoutBraces);
    displayName = json["display_name"].toString();
    username = json["username"].toString();
    avatarUrl = json["avatar"].toString();
    id = json["id"].toString();
}

QUrl MastodonAccount::getWebSocketUrl() {
    auto accessToken = this->app->oauth2->token();
    qDebug() << "access token: " << accessToken;
    QUrl url = QUrl(this->app->baseUrl);
    url.setScheme("wss");
    // TODO: other streams
    url.setPath("/api/v1/streaming");
    QUrlQuery query;
    query.addQueryItem("stream", "user");
    query.addQueryItem("access_token", accessToken);
    url.setQuery(query);
    qDebug() << "stream url: " << url;
    return url;
}

QString MastodonAccount::fullUsername() {
    if (!this->app) {
        return username;
    }
    return username + "@" + this->app->baseUrl.host();
}
