#include <QJsonDocument>
#include "streamevent.h"

MastodonStreamEvent::MastodonStreamEvent(QObject *parent)
    : QObject{parent}
{}

MastodonStreamEvent::MastodonStreamEvent(QJsonObject json, QObject *parent): QObject{parent} {
    type = json["event"].toString();
    auto payloadString = json["payload"].toString();
    QJsonDocument jsonDoc((QJsonDocument::fromJson(payloadString.toUtf8())));
    QJsonObject jsonObj = jsonDoc.object();
    status = new MastodonStatus(jsonObj, this);
}

bool MastodonStreamEvent::isValid(QJsonObject json) {
    return json["event"].toString() == "update";
}
