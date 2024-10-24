#include "misskeystreamevent.h"

MisskeyStreamEvent::MisskeyStreamEvent(QObject *parent)
    : StreamEvent{parent}
{}


MisskeyStreamEvent::~MisskeyStreamEvent() {

}

MisskeyStatus* MisskeyStreamEvent::getStatus() {
    // TODO
    return nullptr;
}
