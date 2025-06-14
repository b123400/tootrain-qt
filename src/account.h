#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <QObject>
#include <QUrl>
#include <QSettings>
#include <QWebSocket>
#include "streamevent.h"

class Account : public QObject
{
    Q_OBJECT
public:
    explicit Account(QObject *parent = nullptr);
    Account(QSettings *settings, QObject *parent = nullptr);

    // The uuid that is only used in tootrain
    QString uuid;

    QString displayName;
    QString username;
    QString id;
    QString avatarUrl;

    virtual QString fullUsername() = 0;
    virtual void saveToSettings(QSettings *settings);

    virtual QUrl getWebSocketUrl() = 0;
    virtual void connectedToWebSocket(QWebSocket *websocket) = 0;
    virtual StreamEvent* getStreamEventFromWebSocketMessage(QString message) = 0;
    virtual QString serviceName() = 0;
signals:
};

#endif // ACCOUNT_H
