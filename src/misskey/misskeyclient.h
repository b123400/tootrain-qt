#ifndef MISSKEYCLIENT_H
#define MISSKEYCLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include "./misskeyaccount.h"

class MisskeyClient : public QObject
{
    Q_OBJECT
public:
    static MisskeyClient &shared() {static MisskeyClient client; return client;}

    void fetchAuthSession(QUrl host, QString sessionId, std::function<void (MisskeyAccount *)> callback);
    void fetchAntennas(MisskeyAccount *account, std::function<void (QList<MisskeyStreamSource *>)> callback);
    void fetchUserList(MisskeyAccount *account, std::function<void (QList<MisskeyStreamSource *>)> callback);
    void fetchChannels(MisskeyAccount *account, std::function<void (QList<MisskeyStreamSource *>)> callback);

private:
    explicit MisskeyClient(QObject *parent = nullptr);

    QNetworkAccessManager *networkManager = nullptr;

signals:
};

#endif // MISSKEYCLIENT_H
