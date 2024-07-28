#include "status.h"
#include <QJsonArray>

MastodonStatus::MastodonStatus( QObject *parent)
    : QObject{parent}
{
}
MastodonStatus::MastodonStatus(QJsonObject json, QObject *parent): QObject{parent}
{
    id = json["id"].toString();
    content = json["content"].toString();
    account = new MastodonAccount(json["account"].toObject(), this);

    if (json.contains("emojis")) {
        auto emojis = json["emojis"].toArray();
        for (auto emoji : emojis) {
            this->emojis.append(new MastodonEmoji(emoji.toObject(), this));
        }
    }
}

QString MastodonStatus::getText() {
    return this->content;
}

bool MastodonStatus::isEmojisReady() {
    for (auto emoji : emojis) {
        if (!emoji->isReady()) {
            return false;
        }
    }
    return true;
}

void MastodonStatus::downloadEmojis() {
    for (auto emoji : emojis) {
        emoji->download();
    }
}
