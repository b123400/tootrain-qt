#include "misskeyaccount.h"

MisskeyAccount::MisskeyAccount(QJsonObject responseObject, QObject *parent)
    : Account{parent}
{
    id = responseObject["id"].toString();
    name = responseObject["name"].toString();
    username = responseObject["username"].toString();
}

QUrl MisskeyAccount::getWebSocketUrl() {
    return QUrl("TODO");
}

QString MisskeyAccount::fullUsername() {
    return "TODO";
}
void MisskeyAccount::saveToSettings(QSettings *settings) {
    // TODO;
}
