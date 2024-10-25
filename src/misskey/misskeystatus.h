#ifndef MISSKEYSTATUS_H
#define MISSKEYSTATUS_H

#include <QObject>
#include "../status.h"
#include <QJsonObject>
#include "./misskeyaccount.h"
#include "../urlemoji.h"

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

private:
    bool preparedEmoji = false;

    QMap<QString, UrlEmoji*> emojis;
    void prepareEmojis();
};

#endif // MISSKEYSTATUS_H
