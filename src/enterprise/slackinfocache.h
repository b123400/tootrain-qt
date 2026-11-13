#ifndef SLACKINFOCACHE_H
#define SLACKINFOCACHE_H

#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <functional>

// The workspace information a Slack message has to be rendered with: who wrote
// it, what its custom emoji look like, and the names behind the channel and
// user group ids a message mentions. A message arrives with ids only, so this
// is fetched up front: once when a workspace is linked, before the app
// connects, and again in the background on every launch. In between it is
// read from the copy on disk.
//
// The channels are kept for a second reason as well: the channel picker opens
// on them, so that there is a list to read before Slack has answered.
class SlackInfoCache : public QObject
{
    Q_OBJECT
public:
    static SlackInfoCache &shared() {static SlackInfoCache cache; return cache;}

    // Every channel the last fetch could see, as `SlackAPIClient` lists them.
    // What the channel picker shows while it asks Slack for the list again.
    QJsonArray channels(const QString &teamId);

    // Fetches this workspace's users, emoji, channels and user groups and
    // replaces what is on disk. `error` is empty when it has.
    void refresh(const QString &teamId, const QString &accessToken, std::function<void (QString error)> callback);

    // Keeps the cached channels in step with a list the channel picker has
    // just fetched, so that the next picker opens on what Slack last said
    // rather than on what the last launch saw. A workspace with nothing cached
    // yet is left alone: the refresh that fills the rest in brings the
    // channels too.
    void updateChannels(const QJsonArray &channels, const QString &teamId);

    void removeCache(const QString &teamId);

private:
    explicit SlackInfoCache(QObject *parent = nullptr) : QObject{parent} {}

    QJsonObject snapshot(const QString &teamId);
    void store(const QJsonObject &snapshot);
    static QString filePath(const QString &teamId);
};

#endif // SLACKINFOCACHE_H
