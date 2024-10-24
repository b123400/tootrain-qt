#ifndef MISSKEYSTREAMEVENT_H
#define MISSKEYSTREAMEVENT_H

#include <QObject>
#include "../streamevent.h"
#include "./misskeystatus.h"

class MisskeyStreamEvent : public StreamEvent
{
public:
    MisskeyStreamEvent(QObject *parent = nullptr);
    ~MisskeyStreamEvent();

    MisskeyStatus* getStatus();
};

#endif // MISSKEYSTREAMEVENT_H
