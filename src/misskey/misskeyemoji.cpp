#include "misskeyemoji.h"

MisskeyEmoji::MisskeyEmoji(QString shortCode, QUrl hostUrl, QObject *parent)
    : UrlEmoji{QUrl(), parent}
{
    auto emojiUrl = QUrl(hostUrl);
    emojiUrl.setPath("/emoji/" + shortCode + ".webp");
    this->url = emojiUrl;
    this->shortCode = shortCode;
}
