#include "dummystatus.h"

#include "status.h"

DummyStatus::DummyStatus(QString message, QObject *parent)
    : QObject{parent}
{
    this->message = message;
}

QString DummyStatus::getText() {
    return this->message;
}
