#ifndef MASTODONLIST_H
#define MASTODONLIST_H

#include <QObject>
#include <QJsonObject>

class MastodonList : public QObject
{
    Q_OBJECT
public:
    explicit MastodonList(QJsonObject json, QObject *parent = nullptr);

    QString id;
    QString title;

signals:
};

#endif // MASTODONLIST_H
