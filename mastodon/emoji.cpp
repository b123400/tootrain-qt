#include "emoji.h"
#include "../imagemanager.h"

MastodonEmoji::MastodonEmoji(QObject *parent)
    : QObject{parent}
{}

MastodonEmoji::MastodonEmoji(QJsonObject json, QObject *parent)
    : QObject{parent}
{
    shortCode = json["shortcode"].toString();
    staticURL = QUrl(json["static_url"].toString());
    url = QUrl(json["url"].toString());
}


QUrl MastodonEmoji::getUrl() {
    return url;
}

QString MastodonEmoji::getPath() {
    return ImageManager::shared().pathForUrl(this->url);
}

bool MastodonEmoji::isReady() {
    return ImageManager::shared().isReady(this->url);
}

void MastodonEmoji::download() {
    ImageManager::shared().download(this->url);
}
