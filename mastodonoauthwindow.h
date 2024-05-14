#ifndef MASTODONOAUTHWINDOW_H
#define MASTODONOAUTHWINDOW_H

#include <QDialog>
#include <QtNetworkAuth/qoauth2authorizationcodeflow.h>

#include "mastodon/client.h"

namespace Ui {
class MastodonOauthWindow;
}

class MastodonOauthWindow : public QDialog
{
    Q_OBJECT

public:
    explicit MastodonOauthWindow(QWidget *parent = nullptr);
    ~MastodonOauthWindow();

    QNetworkReply *requestHotThreads();

public slots:
    void myaccept();
    void myreject();
    void grant();
    void authenticated();
    void statusChanged();
    void oauthError(const QAbstractOAuth::Error err);
    void oauthErrorMore(const QString &error, const QString &errorDescription, const QUrl &uri);
    void stateChanged(const QString &state);
    void tokenChanged(const QString &state);
    void replyDataReceived(const QByteArray &data);
    void finished(QNetworkReply *reply);
    void networkFinished(QNetworkReply *reply);
signals:
    // void authenticated();

private:
    Ui::MastodonOauthWindow *ui;
    QOAuth2AuthorizationCodeFlow oauth2;
    Mastodon::Client *mastodonClient;
    Mastodon::App *app;

    void updateTokenParams(QAbstractOAuth::Stage stage, QMultiMap<QString, QVariant>* parameters);
};

#endif // MASTODONOAUTHWINDOW_H
