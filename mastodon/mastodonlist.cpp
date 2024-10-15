#include "mastodonlist.h"

MastodonList::MastodonList(QJsonObject json, QObject *parent)
    : QObject{parent}
{
    id = json["id"].toString();
    title = json["title"].toString();
}
