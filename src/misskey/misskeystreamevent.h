#ifndef MISSKEYSTREAMEVENT_H
#define MISSKEYSTREAMEVENT_H

#include <QObject>
#include "../streamevent.h"
#include "./misskeystatus.h"
#include <QJsonObject>

class MisskeyStreamEvent : public StreamEvent
{
public:
    MisskeyStreamEvent(QJsonObject json, QUrl hostUrl, QObject *parent = nullptr);
    ~MisskeyStreamEvent();

    MisskeyStatus* getStatus();

    bool isValid();

private:
    MisskeyStatus *status;
};

#endif // MISSKEYSTREAMEVENT_H
