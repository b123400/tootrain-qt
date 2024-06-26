#ifndef MASTODONACCOUNT_H
#define MASTODONACCOUNT_H

#include <QObject>
#include <QJsonObject>
#include "../account.h"
#include "app.h"

class MastodonAccount : public Account
{
    Q_OBJECT
public:
    explicit MastodonAccount(QObject *parent = nullptr);

    MastodonAccount(QJsonObject json, QObject *parent = nullptr);

    // Only available for accounts from settings, not available for other users
    MastodonApp *app = nullptr;

    QUrl getWebSocketUrl();
    QString fullUsername();

signals:
};

#endif // MASTODONACCOUNT_H
