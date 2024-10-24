#ifndef STREAMEVENT_H
#define STREAMEVENT_H

#include <QObject>
#include "status.h"

class StreamEvent : public QObject
{
    Q_OBJECT
public:
    StreamEvent(QObject *parent = nullptr);
    ~StreamEvent();

    virtual Status* getStatus() = 0;

signals:
};

#endif // STREAMEVENT_H
