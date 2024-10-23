#ifndef MISSKEYACCOUNT_H
#define MISSKEYACCOUNT_H

#include <QObject>
#include "../account.h"
#include <QJsonObject>

class MisskeyAccount : public Account
{
    Q_OBJECT
public:
    MisskeyAccount(QJsonObject responseObject, QObject *parent = nullptr);

    QString id;
    QString name;
    QString username;

    // Only available for accounts from settings, not available for other users
    QString accessToken;

    QUrl getWebSocketUrl() override;
    QString fullUsername() override;
    void saveToSettings(QSettings *settings) override;
};

#endif // MISSKEYACCOUNT_H