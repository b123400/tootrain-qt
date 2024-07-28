#ifndef MASTODONSTATUS_H
#define MASTODONSTATUS_H

#include <QObject>
#include "account.h"
#include "../status.h"
#include "emoji.h"

class MastodonStatus : public QObject, public Status
{
    Q_OBJECT
public:
    explicit MastodonStatus(QObject *parent = nullptr);
    MastodonStatus(QJsonObject json, QObject *parent = nullptr);

    QString id;
    QString content;
    MastodonAccount *account = nullptr;
    QList<MastodonEmoji*> emojis;

    QString getText();
    bool isEmojisReady();
    void downloadEmojis();


signals:
};

#endif // MASTODONSTATUS_H
