#ifndef MISSKEYACCOUNT_H
#define MISSKEYACCOUNT_H

#include <QObject>
#include "../account.h"
#include "../streamevent.h"
#include <QJsonObject>
#include "./misskeystreamsource.h"

class MisskeyAccount : public Account
{
    Q_OBJECT
public:
    MisskeyAccount(QJsonObject responseObject, QUrl baseUrl, QObject *parent = nullptr);
    MisskeyAccount(QSettings *settings, QObject *parent = nullptr);

    // Only available for accounts from settings, not available for other users
    QString accessToken;
    QUrl baseUrl;
    QList<MisskeyStreamSource *> sources;

    QUrl getWebSocketUrl() override;
    void connectedToWebSocket(QWebSocket *websocket) override;
    QString fullUsername() override;
    void saveToSettings(QSettings *settings) override;
    QString serviceName() override;

    StreamEvent* getStreamEventFromWebSocketMessage(QString message) override;
};

#endif // MISSKEYACCOUNT_H
