#include "app.h"
#include <QJsonObject>


MastodonApp::MastodonApp(QObject *parent)
    : QObject{parent}
{}

MastodonApp::MastodonApp(QJsonObject json, QUrl baseUrl, QObject *parent): QObject{parent}
{
    clientId = json["client_id"].toString();
    clientSecret = json["client_secret"].toString();
    redirectUri = json["redirect_uri"].toString();
    this->baseUrl = baseUrl;
}

