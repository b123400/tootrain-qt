#ifndef EMOJI_H
#define EMOJI_H

#include <QObject>
#include <QUrl>
#include <QJsonObject>
#include "../urlemoji.h"

class MastodonEmoji : public UrlEmoji
{
    Q_OBJECT
public:
    MastodonEmoji(QJsonObject json, QObject *parent = nullptr);

    QString shortCode;
    QUrl staticURL;

signals:
};

#endif // EMOJI_H
