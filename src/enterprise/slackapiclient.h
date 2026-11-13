#ifndef SLACKAPICLIENT_H
#define SLACKAPICLIENT_H

#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QObject>
#include <QUrlQuery>
#include <functional>

// The slice of Slack's Web API the app needs: who it is talking as, the
// channels it may be let into, and the workspace information a rain drop has
// to be rendered with. Every call is made with one workspace's access token.
//
// A callback gets what Slack answered, or else a sentence for the user saying
// why there is nothing.
class SlackAPIClient : public QObject
{
    Q_OBJECT
public:
    typedef std::function<void (QJsonObject answer, QString error)> ObjectCallback;
    typedef std::function<void (QJsonArray items, QString error)> ListCallback;

    static SlackAPIClient &shared() {static SlackAPIClient client; return client;}

    // `auth.test` as Slack answers it: `team_id`, `team`, `user_id`, `user`, `url`.
    void identity(const QString &accessToken, ObjectCallback callback);

    // `{id, name, realName, displayName, avatarURL}` for every member.
    void users(const QString &accessToken, ListCallback callback);

    // `{name, url}` for every custom emoji, the aliases followed.
    void emoji(const QString &accessToken, ListCallback callback);

    // `{id, name, isPrivate, isMember}` for every channel that is not
    // archived. `isMember` is whether the Slack account the app logged in with
    // is in the channel: the TootRain server only grants a channel to someone
    // who is in it, so one that is false is not a channel the app can be given.
    void conversations(const QString &accessToken, ListCallback callback);

    // `{id, name, handle}` for the workspace's user groups, the disabled ones
    // included: a message that mentions a group that has since been turned off
    // still has to be shown with the group's name.
    void userGroups(const QString &accessToken, ListCallback callback);

    // `emoji.list` answers with either a URL or `alias:other-emoji`; follow
    // the aliases so every name the app caches points at an image.
    static QJsonArray resolveEmojiAliases(const QJsonObject &raw);

private:
    explicit SlackAPIClient(QObject *parent = nullptr);

    void get(const QString &accessToken, const QString &method, const QUrlQuery &query, ObjectCallback callback);
    void getPages(const QString &accessToken, const QString &method, const QUrlQuery &query, const QString &key,
                  const QString &cursor, QJsonArray collected, ListCallback callback);

    QNetworkAccessManager *networkManager = nullptr;
};

#endif // SLACKAPICLIENT_H
