#include "emoji.h"

MastodonEmoji::MastodonEmoji(QJsonObject json, QObject *parent)
    : UrlEmoji(QUrl(json["url"].toString()), parent)
{
    shortCode = json["shortcode"].toString();
    staticURL = QUrl(json["static_url"].toString());
}
