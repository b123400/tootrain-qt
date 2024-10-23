#ifndef STATUS_H
#define STATUS_H

#include <QObject>
#include "richtextcomponent.h"

class Status
{
public:
    virtual QString getText() = 0;

    virtual bool isEmojisReady() = 0;
    virtual void downloadEmojis() = 0;
    virtual QList<RichTextComponent*> richTextcomponents() = 0;
};

#endif // STATUS_H