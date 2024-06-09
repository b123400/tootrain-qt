#ifndef DUMMYSTATUS_H
#define DUMMYSTATUS_H

#include <QObject>
#include "status.h"

class DummyStatus : public QObject, public Status
{
    Q_OBJECT
public:
    explicit DummyStatus(QString message, QObject *parent = nullptr);

    QString getText();

private:
    QString message;

signals:
};

#endif // DUMMYSTATUS_H
