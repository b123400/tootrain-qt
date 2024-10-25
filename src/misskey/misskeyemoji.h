#ifndef MISSKEYEMOJI_H
#define MISSKEYEMOJI_H

#include "../urlemoji.h"

class MisskeyEmoji : public UrlEmoji
{
public:
    MisskeyEmoji(QString shortCode, QUrl hostUrl, QObject *parent = nullptr);
    QString shortCode;
};

#endif // MISSKEYEMOJI_H
