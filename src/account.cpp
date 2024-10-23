#include "account.h"

Account::Account(QObject *parent)
    : QObject{parent}
{}

Account::Account(QSettings *settings, QObject *parent)
    : QObject{parent}
{
    this->uuid = settings->value("uuid").toString();
    this->displayName = settings->value("displayName").toString();
    this->username = settings->value("username").toString();
    this->id = settings->value("id").toString();
    this->avatarUrl= settings->value("avatarUrl").toString();
}

void Account::saveToSettings(QSettings *settings) {
    settings->setValue("uuid", this->uuid);
    settings->setValue("displayName", this->displayName);
    settings->setValue("username", this->username);
    settings->setValue("id", this->id);
    settings->setValue("avatarUrl", this->avatarUrl);
}
