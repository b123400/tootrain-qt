#include "status.h"

MastodonStatus::MastodonStatus( QObject *parent)
    : QObject{parent}
{
}
MastodonStatus::MastodonStatus(QJsonObject json, QObject *parent): QObject{parent}
{
    id = json["id"].toString();
    content = json["content"].toString();
    account = new MastodonAccount(json["account"].toObject(), this);
}

QString MastodonStatus::getText() {
    return this->content;
}
