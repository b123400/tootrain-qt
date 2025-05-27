#ifndef MASTODONACCOUNT_H
#define MASTODONACCOUNT_H

#include <QObject>
#include <QJsonObject>
#include <QSettings>
#include "../account.h"
#include "app.h"
#include "../streamevent.h"

class MastodonAccount : public Account
{
    Q_OBJECT
public:
    enum Source: int {
        Public = 0,
        PublicLocal = 1,
        PublicRemote = 2,
        Hashtag = 3,
        HashtagLocal = 4,
        User = 5,
        UserNotification = 6,
        List = 7,
        Direct = 8,
    };

    explicit MastodonAccount(QObject *parent = nullptr);

    MastodonAccount(QJsonObject json, QObject *parent = nullptr);
    MastodonAccount(QSettings *settings, QObject *parent = nullptr);

    // Only available for accounts from settings, not available for other users
    MastodonApp *app = nullptr;
    Source source;
    QString hashtag;
    QString listId;
    QString listName;

    QUrl getWebSocketUrl() override;
    void connectedToWebSocket(QWebSocket *websocket) override;
    QString fullUsername() override;
    void saveToSettings(QSettings *settings) override;
    QString serviceName() override;

    StreamEvent* getStreamEventFromWebSocketMessage(QString message) override;

private:
    QString queryParamForSource(Source source);

signals:
};

#endif // MASTODONACCOUNT_H
