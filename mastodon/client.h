#ifndef MASTODONCLIENT_H
#define MASTODONCLIENT_H

#include "QtNetwork/qnetworkaccessmanager.h"
#include <QObject>
#include "app.h"
#include "account.h"

class MastodonClient : public QObject
{
    Q_OBJECT
public:
    static MastodonClient &shared() {static MastodonClient client; return client;}

    void createApp(QUrl host, std::function<void (MastodonApp*)> callback);
    void verifyCredentials(MastodonApp *app, QString accessToken, std::function<void (MastodonAccount*)> callback);

signals:

public slots:

private:
    explicit MastodonClient (QObject *parent = nullptr);

    QNetworkAccessManager *networkManager;
};

#endif // MASTODONCLIENT_H
