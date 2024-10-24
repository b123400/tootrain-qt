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
    source = Source::User; // default
}

MastodonAccount::MastodonAccount(QSettings *settings, QObject *parent) : Account(settings, parent) {
    MastodonApp *app = new MastodonApp(this);
    this->app = app;
    QOAuth2AuthorizationCodeFlow *oauth2 = new QOAuth2AuthorizationCodeFlow(app);
    app->oauth2 = oauth2;
    app->clientId = settings->value("clientId").toString();
    oauth2->setClientIdentifier(app->clientId);
    oauth2->setToken(settings->value("accessToken").toString());
    app->clientSecret = settings->value("clientSecret").toString();
    app->redirectUri = settings->value("redirectUri").toString();
    app->baseUrl = settings->value("baseUrl").toUrl();

    if (settings->contains("source")) {
        bool sourceOk = false;
        int source_int = settings->value("source").toInt(&sourceOk);
        if (!sourceOk) {
            this->source = Source::User;
        } else {
            this->source = static_cast<Source>(source_int);
        }
        hashtag = settings->value("hashtag").toString();
        listId = settings->value("listId").toString();
        listName = settings->value("listName").toString();
    } else {
        source = Source::User;
        hashtag = "";
        listId = "";
        listName = "";
    }
}

QUrl MastodonAccount::getWebSocketUrl() {
    auto accessToken = this->app->oauth2->token();
    qDebug() << "access token: " << accessToken;
    QUrl url = QUrl(this->app->baseUrl);
    url.setScheme("wss");
    url.setPath("/api/v1/streaming");
    QUrlQuery query;
    query.addQueryItem("stream", this->queryParamForSource(this->source));
    switch (this->source) {
    case Hashtag:
    case HashtagLocal:
        query.addQueryItem("tag", this->hashtag);
        break;
    case List:
        query.addQueryItem("list", this->listId);
        break;
    default:
        break;
    }

    query.addQueryItem("access_token", accessToken);
    url.setQuery(query);
    qDebug() << "stream url: " << url;
    return url;
}

void MastodonAccount::connectedToWebSocket(QWebSocket *websocket) {

}

QString MastodonAccount::fullUsername() {
    if (!this->app) {
        return username;
    }
    return username + "@" + this->app->baseUrl.host();
}

void MastodonAccount::saveToSettings(QSettings *settings) {
    this->Account::saveToSettings(settings);
    settings->setValue("type", "mastodon");
    settings->setValue("clientId", this->app->clientId);
    settings->setValue("clientSecret", this->app->clientSecret);
    settings->setValue("redirectUri", this->app->redirectUri);
    settings->setValue("baseUrl", this->app->baseUrl);
    settings->setValue("accessToken", this->app->oauth2->token());

    settings->setValue("source", static_cast<int>(this->source));
    settings->setValue("hashtag", this->hashtag);
    settings->setValue("listId", this->listId);
    settings->setValue("listName", this->listName);
}

QString MastodonAccount::queryParamForSource(Source source) {
    switch (source) {
    case Public:
        return "public";
    case PublicLocal:
        return "public:local";
    case PublicRemote:
        return "public:remote";
    case Hashtag:
        return "hashtag";
    case HashtagLocal:
        return "hashtag:local";
    case User:
        return "user";
    case UserNotification:
        return "user:notification";
    case List:
        return "list";
    case Direct:
        return "direct";
    }
}
