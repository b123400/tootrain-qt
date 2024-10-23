#ifndef MISSKEYCLIENT_H
#define MISSKEYCLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include "./misskeywellknown.h"
#include "./misskeyaccount.h"

class MisskeyClient : public QObject
{
    Q_OBJECT
public:
    static MisskeyClient &shared() {static MisskeyClient client; return client;}

    void fetchWellKnown(QUrl host, std::function<void (MisskeyWellKnown *)> callback);
    void fetchAccountWithToken(QUrl host, QString token, std::function<void (MisskeyAccount *)> callback);

private:
    explicit MisskeyClient(QObject *parent = nullptr);

    QNetworkAccessManager *networkManager = nullptr;

signals:
};

#endif // MISSKEYCLIENT_H
