#ifndef MASTODONAPP_H
#define MASTODONAPP_H

#include <QObject>
#include <QUrl>


class MastodonApp : public QObject
{
    Q_OBJECT
public:
    explicit MastodonApp(QObject *parent = nullptr);
    MastodonApp(QJsonObject json, QUrl baseUrl, QObject *parent = nullptr);

    QString clientId;
    QString clientSecret;
    QString redirectUri;
    QUrl baseUrl;

signals:
};

#endif // MASTODONAPP_H
