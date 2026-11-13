#include "slackinfocache.h"

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QSaveFile>
#include <QStandardPaths>
#include <QUrl>

#include "slackapiclient.h"

QJsonArray SlackInfoCache::channels(const QString &teamId) {
    return snapshot(teamId).value("channels").toArray();
}

void SlackInfoCache::refresh(const QString &teamId, const QString &accessToken, std::function<void (QString error)> callback) {
    if (accessToken.isEmpty()) {
        callback(tr("This Slack workspace has no access token, please log in again."));
        return;
    }
    SlackAPIClient &client = SlackAPIClient::shared();
    client.users(accessToken, [=, &client](QJsonArray users, QString error) {
        if (!error.isEmpty()) { callback(error); return; }
        client.emoji(accessToken, [=, &client](QJsonArray emoji, QString error) {
            if (!error.isEmpty()) { callback(error); return; }
            // The channels and the user groups are names to put to the ids in
            // a message, which is worth having but not worth losing the rest
            // of the workspace over: a token granted before the server asked
            // for these scopes is refused them, and so is a workspace on a
            // Slack plan that has no user groups at all. Whichever is refused
            // is kept as it was, and the rest is cached either way.
            client.conversations(accessToken, [=, &client](QJsonArray channels, QString error) {
                const QJsonObject cached = snapshot(teamId);
                if (!error.isEmpty()) {
                    qDebug() << "TootRain: could not list the channels of" << teamId << "keeping the cached ones:" << error;
                    channels = cached["channels"].toArray();
                }
                client.userGroups(accessToken, [=](QJsonArray userGroups, QString error) {
                    if (!error.isEmpty()) {
                        qDebug() << "TootRain: could not list the user groups of" << teamId << "keeping the cached ones:" << error;
                        userGroups = cached["userGroups"].toArray();
                    }
                    store(QJsonObject{
                        {"teamId", teamId},
                        {"updatedAt", QDateTime::currentDateTimeUtc().toString(Qt::ISODate)},
                        {"users", users},
                        {"emoji", emoji},
                        {"channels", channels},
                        {"userGroups", userGroups},
                    });
                    qDebug() << "TootRain: cached" << users.size() << "Slack users," << emoji.size() << "emoji,"
                             << channels.size() << "channels and" << userGroups.size() << "user groups for" << teamId;
                    callback(QString());
                });
            });
        });
    });
}

void SlackInfoCache::updateChannels(const QJsonArray &channels, const QString &teamId) {
    QJsonObject current = snapshot(teamId);
    if (current.isEmpty()) return;
    current["channels"] = channels;
    store(current);
}

void SlackInfoCache::removeCache(const QString &teamId) {
    QFile::remove(filePath(teamId));
}

QJsonObject SlackInfoCache::snapshot(const QString &teamId) {
    QFile file(filePath(teamId));
    if (!file.open(QIODevice::ReadOnly)) return QJsonObject();
    return QJsonDocument::fromJson(file.readAll()).object();
}

void SlackInfoCache::store(const QJsonObject &snapshot) {
    QString path = filePath(snapshot["teamId"].toString());
    QDir().mkpath(QFileInfo(path).path());
    // Written beside the old copy and swapped in, so a write that is cut short
    // leaves the old one rather than half of the new one.
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)
        || file.write(QJsonDocument(snapshot).toJson(QJsonDocument::Compact)) < 0
        || !file.commit()) {
        qDebug() << "TootRain: could not write the Slack cache for" << snapshot["teamId"].toString() << file.errorString();
    }
}

// The cache directory, not the settings: every byte of this can be fetched
// from Slack again, and is, on the next launch, so the system is welcome to
// throw it away whenever it wants the space back.
QString SlackInfoCache::filePath(const QString &teamId) {
    // Percent-encoded, so an id can only ever name a file inside the directory.
    return QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
        + "/SlackCache/" + QString::fromLatin1(QUrl::toPercentEncoding(teamId)) + ".json";
}
