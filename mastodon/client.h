#ifndef MASTODONCLIENT_H
#define MASTODONCLIENT_H

#include "QtNetwork/qnetworkaccessmanager.h"
#include <QObject>
#include "app.h"

namespace Mastodon {
class Client : public QObject
{
    Q_OBJECT
public:
    explicit Client(QObject *parent = nullptr);

    void createApp(QUrl host, std::function<void (App*)> callback);

signals:

public slots:

};
}

#endif // MASTODONCLIENT_H
