#include "statusimageloader.h"
#include "./imagemanager.h"

StatusImageLoader::StatusImageLoader(QObject *parent)
    : QObject{parent}
{
    connect(&ImageManager::shared(), &ImageManager::imageFinishedDownloading, this, &StatusImageLoader::imageFinishedDownloading);
}

void StatusImageLoader::loadEmojisForStatus(Status *status) {
    if (status->isEmojisReady()) {
        emit statusIsReady(status);
        return;
    }

    status->downloadEmojis();
    pendingStatuses.append(status);
}

void StatusImageLoader::imageFinishedDownloading(QUrl url) {
    QMutableListIterator<Status *> i(pendingStatuses);
    while (i.hasNext()) {
        Status *status = i.next();
        if (status->isEmojisReady()) {
            emit statusIsReady(status);
            i.remove();
        }
    }
}
