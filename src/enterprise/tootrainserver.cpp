#include "tootrainserver.h"

#include <QJsonDocument>
#include <QNetworkReply>
#include <QTimer>

#include "../settingmanager.h"

QUrl TootRainServer::serverUrl() {
    QString replaced = SettingManager::shared().value("tootRainServerURL").toString();
    if (!replaced.isEmpty()) return QUrl(replaced);
#ifdef QT_NO_DEBUG
    return QUrl("https://tootrain.b123400.net");
#else
    return QUrl("https://tootrain-staging.b123400.net");
#endif
}

QUrl TootRainServer::centrifugoWebSocketUrl() {
    QString replaced = SettingManager::shared().value("tootRainCentrifugoURL").toString();
#ifdef QT_NO_DEBUG
    QUrl url("https://tootrain-stream.b123400.net");
#else
    QUrl url("https://tootrain-staging-stream.b123400.net");
#endif
    if (!replaced.isEmpty()) url = QUrl(replaced);
    url.setScheme(url.scheme() == "http" ? "ws" : "wss");
    url.setPath("/connection/websocket");
    url.setQuery(QString());
    return url;
}

QUrl TootRainServer::logInWithSlackUrl() {
    QUrl url = serverUrl();
    url.setPath("/users/log-in-with-slack");
    return url;
}

// Filed under the server it belongs to, so a staging session is never offered
// to production.
static QString sessionTokenKey() {
    return "tootRainSessionToken/" + TootRainServer::serverUrl().host();
}

QString TootRainServer::sessionToken() {
    return SettingManager::shared().value(sessionTokenKey()).toString();
}

void TootRainServer::setSessionToken(const QString &token) {
    SettingManager::shared().setValue(sessionTokenKey(), token.isEmpty() ? QVariant() : QVariant(token));
}

TootRainServerAPI::TootRainServerAPI(QObject *parent)
    : QObject{parent}
{
    networkManager = new QNetworkAccessManager(this);
}

void TootRainServerAPI::session(Callback callback) {
    send("GET", "/api/session", QUrlQuery(), QByteArray(), "slack/linkages", callback);
}

void TootRainServerAPI::deleteLinkage(int id, Callback callback) {
    send("DELETE", "/api/slack/linkages/" + QString::number(id), QUrlQuery(), QByteArray(), QString(), callback);
}

void TootRainServerAPI::channelMemberships(const QString &teamId, Callback callback) {
    QUrlQuery query;
    query.addQueryItem("team_id", teamId);
    send("GET", "/api/slack/channel_memberships", query, QByteArray(), "memberships", callback);
}

void TootRainServerAPI::replaceChannelMemberships(const QJsonArray &pairs, Callback callback) {
    QByteArray body = QJsonDocument(QJsonObject{{"memberships", pairs}}).toJson(QJsonDocument::Compact);
    send("PUT", "/api/slack/channel_memberships", QUrlQuery(), body, "memberships", callback);
}

QJsonArray TootRainServerAPI::ofLinkage(const QJsonArray &entries, int linkageId) {
    QJsonArray result;
    for (const QJsonValue &entry : entries) {
        if (entry["linkage_id"].toInt() == linkageId) result.append(entry);
    }
    return result;
}

QVariantMap TootRainServerAPI::channelNames(const QJsonArray &memberships) {
    QVariantMap channels;
    for (const QJsonValue &membership : memberships) {
        QString channelId = membership["channel_id"].toString();
        QString name = membership["name"].toString();
        channels[channelId] = name.isEmpty() ? channelId : name;
    }
    return channels;
}

QString TootRainServerAPI::errorNamed(const QString &code, const QString &detail) {
    if (code == "workspace_not_found") return tr("The TootRain server does not know this Slack workspace.");
    if (code == "not_linked") return tr("This Slack workspace is not linked to this TootRain session, please log in again.");
    if (code == "not_a_member") return tr("You are not a member of this Slack channel, please join it in Slack first.");
    if (code == "bot_not_in_channel") return tr("Please invite the TootRain bot to this channel first.");
    if (code == "slack_error") return tr("Slack refused the request: %1").arg(detail.isEmpty() ? "unknown" : detail);
    if (code == "upstream_error") return tr("The TootRain server could not reach Slack, please try again later.");
    // `missing_params` and `invalid_membership` are this app's own bug rather
    // than anything the user can do something about, and are reported by name
    // for want of anything better to say.
    return tr("The TootRain server refused the request: %1").arg(code);
}

// `requiredArray` is the path of an array the answer has to carry to be worth
// acting on: an answer without it is refused here, rather than read further on
// as a session that holds nothing.
void TootRainServerAPI::send(const QByteArray &method, const QString &path, const QUrlQuery &query,
                             const QByteArray &body, const QString &requiredArray, Callback callback) {
    QString token = TootRainServer::sessionToken();
    if (token.isEmpty()) {
        QTimer::singleShot(0, this, [=]() {
            callback(QJsonObject(), tr("Please log in to TootRain with Slack first."));
        });
        return;
    }
    QUrl url = TootRainServer::serverUrl();
    url.setPath(path);
    url.setQuery(query);
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", "Bearer " + token.toUtf8());
    request.setRawHeader("Accept", "application/json");
    request.setHeader(QNetworkRequest::UserAgentHeader, TootRainServer::userAgent());
    if (!body.isEmpty()) {
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    }

    QNetworkReply *reply = networkManager->sendCustomRequest(request, method, body);
    connect(reply, &QNetworkReply::finished, this, [=]() {
        reply->deleteLater();
        int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QByteArray data = reply->readAll();
        QJsonDocument document = QJsonDocument::fromJson(data);
        if (status == 0) {
            // It never reached the server.
            callback(QJsonObject(), reply->errorString());
            return;
        }
        if (status >= 200 && status < 300) {
            QJsonValue required = document.object();
            for (const QString &key : requiredArray.split('/', Qt::SkipEmptyParts)) {
                required = required[key];
            }
            if (!document.isObject() || (!requiredArray.isEmpty() && !required.isArray())) {
                callback(QJsonObject(), tr("The server sent a response TootRain could not read."));
            } else {
                callback(document.object(), QString());
            }
            return;
        }
        // One of the server's refusals is `{"error": "<code>"}`, with a
        // `detail` when the code alone does not say enough.
        const QJsonObject refusal = document.object();
        if (!refusal["error"].isString()) {
            callback(QJsonObject(), tr("The TootRain server answered with %1: %2").arg(status).arg(QString::fromUtf8(data)));
        } else if (status == 401) {
            // A dead session is answered with a sentence rather than a code,
            // so the status is what names that one.
            callback(QJsonObject(), tr("This TootRain session is no longer valid, please log in again."));
        } else {
            callback(QJsonObject(), errorNamed(refusal["error"].toString(), refusal["detail"].toString()));
        }
    });
}
