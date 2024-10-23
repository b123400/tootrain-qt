#ifndef STATUSEMOJI_H
#define STATUSEMOJI_H

#include <QObject>

class StatusEmoji
{
public:
    virtual QString getPath() = 0;
};

#endif // STATUSEMOJI_H
