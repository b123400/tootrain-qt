#ifndef MASTODONSTREAMEVENT_H
#define MASTODONSTREAMEVENT_H

#include <QObject>
#include "../streamevent.h"
#include "status.h"

class MastodonStreamEvent : public StreamEvent
{
public:
    MastodonStreamEvent(QObject *parent = nullptr);
    ~MastodonStreamEvent();

    MastodonStreamEvent(QJsonObject json, QObject *parent = nullptr);
    static bool isValid(QJsonObject json);

    QString type;
    MastodonStatus *status = nullptr;

    MastodonStatus* getStatus();

signals:
};

#endif // MASTODONSTREAMEVENT_H
