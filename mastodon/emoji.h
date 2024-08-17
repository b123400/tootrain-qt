#ifndef EMOJI_H
#define EMOJI_H

#include <QObject>
#include <QUrl>
#include <QJsonObject>
#include "../statusemoji.h"

class MastodonEmoji : public QObject, public StatusEmoji
{
    Q_OBJECT
public:
    explicit MastodonEmoji(QObject *parent = nullptr);
    MastodonEmoji(QJsonObject json, QObject *parent = nullptr);

    QString shortCode;
    QUrl staticURL;
    QUrl url;

    QUrl getUrl();
    QString getPath();

    bool isReady();
    void download();

signals:
};

#endif // EMOJI_H
