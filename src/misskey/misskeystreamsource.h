#ifndef MISSKEYSTREAMSOURCE_H
#define MISSKEYSTREAMSOURCE_H

#include <QObject>
#include <QSettings>

class MisskeyStreamSource : public QObject
{
    Q_OBJECT
public:
    enum Channel: int {
        Home = 0,
        Global = 1,
        Hybrid = 2,
        Local = 3,
        Main = 4,
        Antenna = 5,
        UserList = 6,
        Channel = 7,
    };

    MisskeyStreamSource(QSettings *settings, QObject *parent = nullptr);
    MisskeyStreamSource(enum Channel channel, QObject *parent = nullptr);

    enum Channel channel;

    QString antennaId;
    QString antennaName;
    QString userListId;
    QString userListName;
    QString channelId;
    QString channelName;

    void saveToSettings(QSettings *settings);
    QString displayName();
    bool contentEqual(MisskeyStreamSource *a);

    MisskeyStreamSource* copy(QObject *newParent = nullptr);

    static QList<MisskeyStreamSource*> defaultSources(QObject *parent = nullptr);
signals:
};

#endif // MISSKEYSTREAMSOURCE_H
