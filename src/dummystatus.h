#ifndef DUMMYSTATUS_H
#define DUMMYSTATUS_H

#include <QObject>
#include "status.h"

class DummyStatus : public Status
{
public:
    explicit DummyStatus(QString message, QObject *parent = nullptr);

    QString getText();

    bool isEmojisReady();
    void downloadEmojis();
    QList<RichTextComponent*> richTextcomponents();

private:
    QString message;

signals:
};

#endif // DUMMYSTATUS_H
