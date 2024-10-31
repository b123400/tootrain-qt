#include "misskeystreamsource.h"

MisskeyStreamSource::MisskeyStreamSource(QSettings *settings, QObject *parent)
    : QObject{parent}
{
    bool channelOk = false;
    int channelInt = settings->value("channel").toInt(&channelOk);
    if (!channelOk) {
        this->channel = Channel::Home;
    } else {
        this->channel = static_cast<enum Channel>(channelInt);
    }
    switch (this->channel) {
    case Channel::Antenna:
        this->antennaId = settings->value("antennaId").toString();
        this->antennaName = settings->value("antennaName").toString();
        break;
    case Channel::UserList:
        this->userListId = settings->value("userListId").toString();
        this->userListName = settings->value("userListName").toString();
        break;
    case Channel::Channel:
        this->channelId = settings->value("channelId").toString();
        this->channelName = settings->value("channelName").toString();
        break;
    default:
        // noop
        break;
    }
}

MisskeyStreamSource::MisskeyStreamSource(enum Channel channel, QObject *parent): QObject{parent} {
    this->channel = channel;
}

QString MisskeyStreamSource::displayName() {
    switch (this->channel) {
    case Home:
        return tr("Home Timeine");
    case Global:
        return tr("Global Timeline");
    case Hybrid:
        return tr("Hybrid Timeline");
    case Local:
        return tr("Local Timeline");
    case Main:
        return tr("Main (Reply and Renote)");
    case Antenna:
        return tr("Antenna - ") + this->antennaName;
    case UserList:
        return tr("List - ") + this->userListName;
    case Channel:
        return tr("Channel - ") + this->channelName;
    }
}

void MisskeyStreamSource::saveToSettings(QSettings *settings) {
    settings->setValue("channel", static_cast<int>(this->channel));
    switch (this->channel) {
    case Channel::Antenna:
        settings->setValue("antennaId", this->antennaId);
        settings->setValue("antennaName", this->antennaName);
        break;
    case Channel::UserList:
        settings->setValue("userListId", this->userListId);
        settings->setValue("userListName", this->userListName);
        break;
    case Channel::Channel:
        settings->setValue("channelId", this->channelId);
        settings->setValue("channelName", this->channelName);
        break;
    default:
        // noop
        break;
    }
}

bool MisskeyStreamSource::contentEqual(MisskeyStreamSource *a) {
    if (this->channel != a->channel) return false;
    switch (this->channel) {
    case Antenna:
        return this->antennaId == a->antennaId && this->antennaName == a->antennaName;
    case UserList:
        return this->userListId == a->userListId && this->userListName == a->userListName;
    case Channel:
        return this->channelId == a->channelId && this->channelName == a->channelName;
    default:
        return true;
    }
}

MisskeyStreamSource* MisskeyStreamSource::copy(QObject *newParent) {
    auto m = new MisskeyStreamSource(this->channel, newParent);
    m->antennaId = this->antennaId;
    m->antennaName = this->antennaName;
    m->userListId = this->userListId;
    m->userListName = this->userListName;
    m->channelId = this->channelId;
    m->channelName = this->channelName;
    return m;
}

QList<MisskeyStreamSource*> MisskeyStreamSource::defaultSources(QObject *parent) {
    QList<MisskeyStreamSource *> list;
    list.append(new MisskeyStreamSource(Channel::Home, parent));
    list.append(new MisskeyStreamSource(Channel::Main, parent));
    list.append(new MisskeyStreamSource(Channel::Local, parent));
    list.append(new MisskeyStreamSource(Channel::Global, parent));
    list.append(new MisskeyStreamSource(Channel::Hybrid, parent));
    return list;
}
