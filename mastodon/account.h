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

    MastodonApp *app;

signals:
};

#endif // MASTODONACCOUNT_H
