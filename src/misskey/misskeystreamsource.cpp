#include "misskeystreamsource.h"

MisskeyStreamSource::MisskeyStreamSource(QSettings *settings, QObject *parent)
    : QObject{parent}
{
    bool sourceOk = false;
    int channelInt = settings->value("source").toInt(&sourceOk);
    if (!sourceOk) {
        this->source = Source::Home;
    } else {
        this->source = static_cast<enum Source>(channelInt);
    }
    switch (this->source) {
    case Source::Antenna:
        this->antennaId = settings->value("antennaId").toString();
        this->antennaName = settings->value("antennaName").toString();
        break;
    case Source::UserList:
        this->userListId = settings->value("userListId").toString();
        this->userListName = settings->value("userListName").toString();
        break;
    case Source::Channel:
        this->channelId = settings->value("channelId").toString();
        this->channelName = settings->value("channelName").toString();
        break;
    default:
        // noop
        break;
    }
}

MisskeyStreamSource::MisskeyStreamSource(enum Source source, QObject *parent): QObject{parent} {
    this->source = source;
}

QString MisskeyStreamSource::displayName() {
    switch (this->source) {
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
    return "";
}

void MisskeyStreamSource::saveToSettings(QSettings *settings) {
    settings->setValue("source", static_cast<int>(this->source));
    switch (this->source) {
    case Source::Antenna:
        settings->setValue("antennaId", this->antennaId);
        settings->setValue("antennaName", this->antennaName);
        break;
    case Source::UserList:
        settings->setValue("userListId", this->userListId);
        settings->setValue("userListName", this->userListName);
        break;
    case Source::Channel:
        settings->setValue("channelId", this->channelId);
        settings->setValue("channelName", this->channelName);
        break;
    default:
        // noop
        break;
    }
}

bool MisskeyStreamSource::contentEqual(MisskeyStreamSource *a) {
    if (this->source != a->source) return false;
    switch (this->source) {
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
    auto m = new MisskeyStreamSource(this->source, newParent);
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
    list.append(new MisskeyStreamSource(Source::Home, parent));
    list.append(new MisskeyStreamSource(Source::Main, parent));
    list.append(new MisskeyStreamSource(Source::Local, parent));
    list.append(new MisskeyStreamSource(Source::Global, parent));
    list.append(new MisskeyStreamSource(Source::Hybrid, parent));
    return list;
}
