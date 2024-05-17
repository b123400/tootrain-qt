#ifndef MASTODONAPP_H
#define MASTODONAPP_H

#include <QDateTime>
#include <QObject>
#include <QUrl>
#include <QtNetworkAuth/qoauth2authorizationcodeflow.h>

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

    QOAuth2AuthorizationCodeFlow *oauth2;

signals:
};

#endif // MASTODONAPP_H
