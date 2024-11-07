#ifndef MASTODONSTATUS_H
#define MASTODONSTATUS_H

#include <QObject>
#include "account.h"
#include "../status.h"
#include "emoji.h"
#include "../richtextcomponent.h"

class MastodonStatus : public Status
{
public:
    explicit MastodonStatus(QObject *parent = nullptr);
    MastodonStatus(QJsonObject json, QObject *parent = nullptr);

    QString id;
    QString content;
    MastodonAccount *account = nullptr;
    QList<MastodonEmoji*> emojis;
    UrlEmoji *avatarEmoji = nullptr;

    QString getText();
    QList<RichTextComponent*> richTextcomponents();

    bool isEmojisReady();
    void downloadEmojis();

    QString getPlainText();

signals:
};

#endif // MASTODONSTATUS_H
