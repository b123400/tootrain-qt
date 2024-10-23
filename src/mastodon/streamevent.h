#ifndef STREAMEVENT_H
#define STREAMEVENT_H

#include <QObject>
#include "status.h"

class MastodonStreamEvent : public QObject
{
    Q_OBJECT
public:
    explicit MastodonStreamEvent(QObject *parent = nullptr);
    MastodonStreamEvent(QJsonObject json, QObject *parent = nullptr);
    static bool isValid(QJsonObject json);

    QString type;
    MastodonStatus *status = nullptr;


signals:
};

#endif // STREAMEVENT_H
