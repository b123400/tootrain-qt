#include "app.h"
#include <QJsonObject>

namespace Mastodon {

App::App(QObject *parent)
    : QObject{parent}
{}

App::App(QJsonObject json, QObject *parent): QObject{parent}
{
    clientId = json["client_id"].toString();
    clientSecret = json["client_secret"].toString();
    redirectUri = json["redirect_uri"].toString();
}

}
