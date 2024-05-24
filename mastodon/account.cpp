#include "account.h"
#include <QUuid>

MastodonAccount::MastodonAccount(QObject *parent)
    : Account{parent}
{}

MastodonAccount::MastodonAccount(QJsonObject json, QObject *parent) : Account {parent}
{
    uuid = QUuid::createUuid().toString(QUuid::StringFormat::WithoutBraces);
    displayName = json["display_name"].toString();
    username = json["username"].toString();
    avatarUrl = json["avatar"].toString();
    id = json["id"].toString();
}
