#ifndef URLEMOJI_H
#define URLEMOJI_H

#include <QObject>
#include <QUrl>
#include "statusemoji.h"

class UrlEmoji : public QObject, public StatusEmoji
{
    Q_OBJECT
public:
    UrlEmoji(QUrl url, QObject *parent = nullptr);

    QUrl url;

    QUrl getUrl();
    QString getPath();

    bool isReady();
    void download();
};

#endif // URLEMOJI_H
