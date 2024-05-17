#include "mastodonoauthwindow.h"
#include "ui_mastodonoauthwindow.h"
#include <QtDebug>
#include <QtNetworkAuth/qoauth2authorizationcodeflow.h>
#include <QtNetworkAuth/qoauthhttpserverreplyhandler.h>
#include <QNetworkProxy>
#include <QPushButton>
#include <QtGui/qdesktopservices.h>
#include "mastodon/client.h"

MastodonOauthWindow::MastodonOauthWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MastodonOauthWindow)
{
    ui->setupUi(this);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &MastodonOauthWindow::myaccept);

    auto replyHandler = new QOAuthHttpServerReplyHandler(12340, this);
    oauth2.setReplyHandler(replyHandler);
    oauth2.setScope("read write");

    oauth2.setModifyParametersFunction([this](QAbstractOAuth::Stage stage, QMultiMap<QString, QVariant>* parameters) {
        this->updateTokenParams(stage, parameters);
    });

    connect(&oauth2, &QOAuth2AuthorizationCodeFlow::granted, this, &MastodonOauthWindow::granted);
    connect(&oauth2, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser, this,
            &QDesktopServices::openUrl);
    connect(&oauth2, &QOAuth2AuthorizationCodeFlow::requestFailed, this, &MastodonOauthWindow::oauthError);
}

MastodonOauthWindow::~MastodonOauthWindow()
{
    delete ui;
}

void MastodonOauthWindow::myaccept() {
    if (app) {
        grant();
        return;
    }
    auto urlStr = this->ui->lineEdit->text();
    MastodonClient::shared().createApp(QUrl(urlStr), [=](MastodonApp* app){
        this->app = app;
        app->setParent(this);
        this->grant();
    });
}

QNetworkReply *MastodonOauthWindow::requestHotThreads()
{
    return oauth2.get(QUrl("https://mastodon.hk/api/v1/timelines/public"));
}

void MastodonOauthWindow::grant()
{
    auto authUrl = QUrl(this->ui->lineEdit->text());
    authUrl.setPath("/oauth/authorize");
    auto tokenUrl = QUrl(this->ui->lineEdit->text());
    tokenUrl.setPath("/oauth/token");
    oauth2.setAuthorizationUrl(authUrl);
    oauth2.setAccessTokenUrl(tokenUrl);
    oauth2.setClientIdentifier(this->app->clientId);
    oauth2.grant();
}

void MastodonOauthWindow::granted() {
    auto token = oauth2.token();
    // oauth2.refreshToken();
    oauth2.expirationAt();
    qDebug() << "token:" << token;
    MastodonClient::shared().verifyCredentials(app, token, [=](MastodonAccount* account){
        account->setParent(this);
        account->app = app;
        app->setParent(account);
        authenticated(account);
    });

    auto okButton = ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok);
    okButton->setEnabled(false);
    // close();
}

void MastodonOauthWindow::oauthError(const QAbstractOAuth::Error err) {
    qDebug() << "oh no";
}

void MastodonOauthWindow::updateTokenParams(QAbstractOAuth::Stage stage, QMultiMap<QString, QVariant>* parameters) {
    if (stage == QAbstractOAuth::Stage::RequestingAccessToken) {
        parameters->insert("client_secret", app->clientSecret);
    }
}
