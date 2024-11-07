#include "misskeystreamevent.h"

MisskeyStreamEvent::MisskeyStreamEvent(QJsonObject json, QUrl hostUrl, QObject *parent)
    : StreamEvent{parent}
{
    if (json["type"].toString() == "channel" && json["body"].isObject()) {
        QJsonObject eventObj = json["body"].toObject();
        QString type = eventObj["type"].toString();
        if ((type == "note" || type == "reply" || type == "mention") && eventObj["body"].isObject()) {
            status = new MisskeyStatus(eventObj["body"].toObject(), hostUrl, this);
        }
    }
}

bool MisskeyStreamEvent::isValid() {
    return this->status != nullptr && !this->status->id.isEmpty();
}

MisskeyStreamEvent::~MisskeyStreamEvent() {

}

MisskeyStatus* MisskeyStreamEvent::getStatus() {
    if (this->status->body.isEmpty()) return nullptr;
    return this->status;
}
