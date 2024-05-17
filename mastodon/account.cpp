#include "account.h"

MastodonAccount::MastodonAccount(QObject *parent)
    : Account{parent}
{}

MastodonAccount::MastodonAccount(QJsonObject json, QObject *parent) : Account {parent}
{
    displayName = json["display_name"].toString();
    username = json["username"].toString();
    avatarUrl = json["avatar"].toString();
    id = json["id"].toString();
}
