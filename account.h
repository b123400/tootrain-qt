#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <QObject>
#include <QUrl>

class Account : public QObject
{
    Q_OBJECT
public:
    explicit Account(QObject *parent = nullptr);

    // The uuid that is only used in tootrain
    QString uuid;

    QString displayName;
    QString username;
    QString id;
    QString avatarUrl;

    virtual QUrl getWebSocketUrl() = 0;
    virtual QString fullUsername() = 0;
signals:
};

#endif // ACCOUNT_H
