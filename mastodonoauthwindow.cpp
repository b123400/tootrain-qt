#include "mastodonoauthwindow.h"
#include "ui_mastodonoauthwindow.h"
#include <QtDebug>
#include <QtNetworkAuth/qoauth2authorizationcodeflow.h>
#include <QtNetworkAuth/qoauthhttpserverreplyhandler.h>
#include <QtGui/qdesktopservices.h>
#include "mastodon/client.h"

MastodonOauthWindow::MastodonOauthWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MastodonOauthWindow)
{
    mastodonClient = new Mastodon::Client(this);
    ui->setupUi(this);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &MastodonOauthWindow::myaccept);

    auto replyHandler = new QOAuthHttpServerReplyHandler(1337, this);
    oauth2.setReplyHandler(replyHandler);
    oauth2.setScope("read write");

    oauth2.setModifyParametersFunction([this](QAbstractOAuth::Stage stage, QMultiMap<QString, QVariant>* parameters) {
        this->updateTokenParams(stage, parameters);
    });

    connect(&oauth2, &QOAuth2AuthorizationCodeFlow::granted, this, &MastodonOauthWindow::authenticated);
    connect(&oauth2, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser, this,
            &QDesktopServices::openUrl);
    connect(&oauth2, &QOAuth2AuthorizationCodeFlow::statusChanged, this, &MastodonOauthWindow::statusChanged);
    connect(&oauth2, &QOAuth2AuthorizationCodeFlow::requestFailed, this, &MastodonOauthWindow::oauthError);
    connect(&oauth2, &QOAuth2AuthorizationCodeFlow::error, this, &MastodonOauthWindow::oauthErrorMore);
    connect(&oauth2, &QOAuth2AuthorizationCodeFlow::stateChanged, this, &MastodonOauthWindow::stateChanged);
    connect(&oauth2, &QOAuth2AuthorizationCodeFlow::tokenChanged, this, &MastodonOauthWindow::tokenChanged);
    connect(&oauth2, &QOAuth2AuthorizationCodeFlow::replyDataReceived, this, &MastodonOauthWindow::replyDataReceived);
    connect(&oauth2, &QOAuth2AuthorizationCodeFlow::finished, this, &MastodonOauthWindow::finished);

}

MastodonOauthWindow::~MastodonOauthWindow()
{
    delete ui;
}

void MastodonOauthWindow::myaccept() {
    qDebug() << "Hello world111!";
    // oauth2.grant();
    auto urlStr = this->ui->plainTextEdit->toPlainText();
    mastodonClient->createApp(QUrl(urlStr), [=](Mastodon::App* app){
        this->app = app;
        this->grant();
    });
}
void MastodonOauthWindow::myreject() {
    qDebug() << "Hello world222!";
}

QNetworkReply *MastodonOauthWindow::requestHotThreads()
{
    return oauth2.get(QUrl("https://mastodon.hk/api/v1/timelines/public"));
}

void MastodonOauthWindow::grant()
{
    auto authUrl = QUrl(this->ui->plainTextEdit->toPlainText());
    authUrl.setPath("/oauth/authorize");
    auto tokenUrl = QUrl(this->ui->plainTextEdit->toPlainText());
    tokenUrl.setPath("/oauth/token");
    oauth2.setAuthorizationUrl(authUrl);
    oauth2.setAccessTokenUrl(tokenUrl);
    oauth2.setClientIdentifier(this->app->clientId);
    oauth2.grant();
}

void MastodonOauthWindow::authenticated() {
    auto tokens = oauth2.token();
    oauth2.refreshToken();
    oauth2.expirationAt();
    qDebug() << "token:" << tokens;

}

void MastodonOauthWindow::statusChanged() {
    auto xxx = oauth2.status();
    qDebug() << "status changed";

}
void MastodonOauthWindow::oauthError(const QAbstractOAuth::Error err) {
    qDebug() << "oh no";
}

void MastodonOauthWindow::oauthErrorMore(const QString &error, const QString &errorDescription, const QUrl &uri) {
    qDebug() << "oh no2" << error;
}

void MastodonOauthWindow::stateChanged(const QString &state) {
    qDebug() << "state changed " << state;
}

void MastodonOauthWindow::tokenChanged(const QString &state) {
    qDebug() << "tokenChanged " << state;
}
void MastodonOauthWindow::replyDataReceived(const QByteArray &data) {
    qDebug() << "replyDataReceived " << QString::fromUtf8(data);
}
void MastodonOauthWindow::finished(QNetworkReply *reply) {
    qDebug() << "finished " << reply;
}

void MastodonOauthWindow::networkFinished(QNetworkReply *reply) {
    qDebug() << "net finished " << reply;
}

void MastodonOauthWindow::updateTokenParams(QAbstractOAuth::Stage stage, QMultiMap<QString, QVariant>* parameters) {
    if (stage == QAbstractOAuth::Stage::RequestingAccessToken) {
        parameters->insert("client_secret", app->clientSecret);
    }
}
