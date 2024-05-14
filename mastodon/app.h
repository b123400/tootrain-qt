#ifndef APP_H
#define APP_H

#include <QObject>

namespace Mastodon {
class App : public QObject
{
    Q_OBJECT
public:
    explicit App(QObject *parent = nullptr);

    QString clientId;
    QString clientSecret;
    QString redirectUri;

    App(QJsonObject json, QObject *parent = nullptr);

signals:
};
}
#endif // APP_H
