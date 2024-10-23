#include "misskeywellknown.h"

MisskeyWellKnown::MisskeyWellKnown(QJsonObject json, QObject *parent)
    : QObject{parent}
{
    authenticateUrl = QUrl(json["authorization_endpoint"].toString());
    accessTokenUrl = QUrl(json["token_endpoint"].toString());
}
