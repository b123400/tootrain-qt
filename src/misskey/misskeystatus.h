#ifndef MISSKEYSTATUS_H
#define MISSKEYSTATUS_H

#include <QObject>
#include "../status.h"
#include <QJsonObject>
#include "./misskeyaccount.h"

class MisskeyStatus : public Status
{
public:
    MisskeyStatus(QJsonObject json, QUrl hostUrl, QObject *parent = nullptr);

    QString id;
    QString body;

    MisskeyAccount *account = nullptr;

    QString getText();

    bool isEmojisReady();
    void downloadEmojis();
    QList<RichTextComponent*> richTextcomponents();
};

#endif // MISSKEYSTATUS_H