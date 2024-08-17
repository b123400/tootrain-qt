#ifndef STATUSIMAGELOADER_H
#define STATUSIMAGELOADER_H

#include "status.h"
#include <QObject>
#include <QSet>

class StatusImageLoader : public QObject
{
    Q_OBJECT
public:
    static StatusImageLoader &shared() {static StatusImageLoader shared; return shared;}
    void loadEmojisForStatus(Status *status);

public slots:
    void imageFinishedDownloading(QUrl url);

private:
    explicit StatusImageLoader(QObject *parent = nullptr);
    QList<Status *> pendingStatuses;

signals:
    void statusIsReady(Status *status);
};

#endif // STATUSIMAGELOADER_H
