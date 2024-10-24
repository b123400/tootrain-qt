#ifndef MISSKEYSTATUS_H
#define MISSKEYSTATUS_H

#include <QObject>
#include "../status.h"

class MisskeyStatus : public Status
{
public:
    MisskeyStatus();

    QString getText();

    bool isEmojisReady();
    void downloadEmojis();
    QList<RichTextComponent*> richTextcomponents();
};

#endif // MISSKEYSTATUS_H
