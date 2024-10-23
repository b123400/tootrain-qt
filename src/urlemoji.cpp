#include "urlemoji.h"
#include "./imagemanager.h"

UrlEmoji::UrlEmoji(QUrl url, QObject *parent): QObject{parent}
{
    this->url = url;
}

QUrl UrlEmoji::getUrl() {
    return url;
}

QString UrlEmoji::getPath() {
    return ImageManager::shared().pathForUrl(this->url);
}

bool UrlEmoji::isReady() {
    return ImageManager::shared().isReady(this->url);
}

void UrlEmoji::download() {
    ImageManager::shared().download(this->url);
}
