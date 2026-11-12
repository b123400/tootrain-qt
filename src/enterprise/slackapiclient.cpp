#include "slackapiclient.h"

#include <QJsonDocument>
#include <QNetworkReply>

#include "tootrainserver.h"

SlackAPIClient::SlackAPIClient(QObject *parent)
    : QObject{parent}
{
    networkManager = new QNetworkAccessManager(this);
}

void SlackAPIClient::identity(const QString &accessToken, ObjectCallback callback) {
    get(accessToken, "auth.test", QUrlQuery(), [=](QJsonObject json, QString error) {
        if (error.isEmpty() && (!json.value("team_id").isString() || !json.value("user_id").isString())) {
            error = tr("Slack sent a response TootRain could not read.");
        }
        callback(json, error);
    });
}

void SlackAPIClient::users(const QString &accessToken, ListCallback callback) {
    getPages(accessToken, "users.list", QUrlQuery(), "members", QString(), QJsonArray(), [=](QJsonArray members, QString error) {
        QJsonArray users;
        for (const QJsonValue &member : members) {
            QString id = member["id"].toString();
            if (id.isEmpty()) continue;
            const QJsonValue profile = member["profile"];
            QJsonObject user{{"id", id}, {"name", member["name"].toString(id)}};
            QString realName = member["real_name"].toString(profile["real_name"].toString());
            if (!realName.isEmpty()) user["realName"] = realName;
            if (!profile["display_name"].toString().isEmpty()) user["displayName"] = profile["display_name"];
            QString avatar = profile["image_192"].toString(profile["image_72"].toString());
            if (!avatar.isEmpty()) user["avatarURL"] = avatar;
            users.append(user);
        }
        callback(users, error);
    });
}

void SlackAPIClient::emoji(const QString &accessToken, ListCallback callback) {
    get(accessToken, "emoji.list", QUrlQuery(), [=](QJsonObject json, QString error) {
        if (error.isEmpty() && !json.value("emoji").isObject()) {
            error = tr("Slack sent a response TootRain could not read.");
        }
        callback(resolveEmojiAliases(json.value("emoji").toObject()), error);
    });
}

void SlackAPIClient::conversations(const QString &accessToken, ListCallback callback) {
    QUrlQuery query;
    query.addQueryItem("types", "public_channel,private_channel");
    query.addQueryItem("exclude_archived", "true");
    getPages(accessToken, "conversations.list", query, "channels", QString(), QJsonArray(), [=](QJsonArray listed, QString error) {
        QJsonArray channels;
        for (const QJsonValue &channel : listed) {
            if (!channel["id"].isString() || !channel["name"].isString()) continue;
            channels.append(QJsonObject{
                {"id", channel["id"]},
                {"name", channel["name"]},
                {"isPrivate", channel["is_private"].toBool(false)},
                {"isMember", channel["is_member"].toBool(true)},
            });
        }
        callback(channels, error);
    });
}

// Slack answers with all of the groups at once, so there is no paging here.
void SlackAPIClient::userGroups(const QString &accessToken, ListCallback callback) {
    QUrlQuery query;
    query.addQueryItem("include_disabled", "true");
    get(accessToken, "usergroups.list", query, [=](QJsonObject json, QString error) {
        if (error.isEmpty() && !json.value("usergroups").isArray()) {
            error = tr("Slack sent a response TootRain could not read.");
        }
        QJsonArray groups;
        for (const QJsonValue &group : json.value("usergroups").toArray()) {
            QString id = group["id"].toString();
            if (id.isEmpty()) continue;
            // The handle is what a mention of the group reads as, and the name
            // is the group's title, which is what is left to call it by when
            // the handle is missing.
            QString handle = group["handle"].toString();
            QJsonObject cached{{"id", id}, {"name", group["name"].toString(handle.isEmpty() ? id : handle)}};
            if (!handle.isEmpty()) cached["handle"] = handle;
            groups.append(cached);
        }
        callback(groups, error);
    });
}

QJsonArray SlackAPIClient::resolveEmojiAliases(const QJsonObject &raw) {
    QJsonArray resolved;
    for (auto it = raw.constBegin(); it != raw.constEnd(); ++it) {
        QString target = it.value().toString();
        // An alias can point at an alias; the hops are counted so that two
        // pointing at each other end rather than go round for ever.
        for (int hops = 0; target.startsWith("alias:") && hops < 10; hops++) {
            QString aliased = target.mid(6);
            if (!raw.contains(aliased)) break;
            target = raw.value(aliased).toString();
        }
        if (target.isEmpty() || target.startsWith("alias:") || !QUrl(target).isValid()) continue;
        resolved.append(QJsonObject{{"name", it.key()}, {"url", target}});
    }
    return resolved;
}

void SlackAPIClient::getPages(const QString &accessToken, const QString &method, const QUrlQuery &query, const QString &key,
                              const QString &cursor, QJsonArray collected, ListCallback callback) {
    QUrlQuery pageQuery = query;
    pageQuery.addQueryItem("limit", "200");
    if (!cursor.isEmpty()) {
        // Percent-encoded here: a cursor is base64, and a `+` left as it is
        // would be read as a space.
        pageQuery.addQueryItem("cursor", QString::fromLatin1(QUrl::toPercentEncoding(cursor)));
    }
    get(accessToken, method, pageQuery, [=](QJsonObject json, QString error) mutable {
        if (!error.isEmpty()) {
            callback(QJsonArray(), error);
            return;
        }
        for (const QJsonValue &item : json.value(key).toArray()) {
            collected.append(item);
        }
        QString next = json.value("response_metadata")["next_cursor"].toString();
        if (next.isEmpty()) {
            callback(collected, QString());
        } else {
            getPages(accessToken, method, query, key, next, collected, callback);
        }
    });
}

void SlackAPIClient::get(const QString &accessToken, const QString &method, const QUrlQuery &query, ObjectCallback callback) {
    QUrl url("https://slack.com/api/" + method);
    url.setQuery(query);
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", "Bearer " + accessToken.toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded; charset=utf-8");
    request.setHeader(QNetworkRequest::UserAgentHeader, TootRainServer::userAgent());

    QNetworkReply *reply = networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [=]() {
        reply->deleteLater();
        int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
        const QJsonObject json = document.object();
        if (status == 0) {
            callback(QJsonObject(), reply->errorString());
        } else if (status < 200 || status >= 300) {
            callback(QJsonObject(), tr("Slack answered with %1.").arg(status));
        } else if (!document.isObject()) {
            callback(QJsonObject(), tr("Slack sent a response TootRain could not read."));
        } else if (!json["ok"].toBool()) {
            callback(QJsonObject(), tr("Slack refused the request: %1").arg(json["error"].toString("unknown")));
        } else {
            callback(json, QString());
        }
    });
}
