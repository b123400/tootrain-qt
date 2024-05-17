#include "client.h"
#include "app.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>

MastodonClient ::MastodonClient (QObject *parent)
    : QObject{parent}
{
    networkManager = new QNetworkAccessManager(this);
}

void MastodonClient ::createApp(QUrl host, std::function<void (MastodonApp*)> callback) {
    QUrl baseUrl = QUrl(host);
    baseUrl.setPath("");
    QUrl url = QUrl(baseUrl);
    url.setPath("/api/v1/apps");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery params;
    params.addQueryItem("client_name", "Tootrain-qt");
    params.addQueryItem("redirect_uris", "http://127.0.0.1:12340/");
    params.addQueryItem("scopes", "read write");
    params.addQueryItem("website", "https://b123400.net/tootrain");

    QNetworkReply *reply = networkManager->post(request, params.query().toUtf8());
    connect(reply, &QNetworkReply::finished, this, [=]()
        {
            // TODO: handle error
            QString err = reply->errorString();
            QByteArray buffer = reply->readAll();
            QJsonDocument jsonDoc((QJsonDocument::fromJson(buffer)));
            QJsonObject jsonReply = jsonDoc.object();
            // qDebug() << "json:" << jsonReply;
            MastodonApp *app = new MastodonApp(jsonReply, baseUrl, this);
            callback(app);
        });
}

void MastodonClient::verifyCredentials(MastodonApp *app, QString accessToken, std::function<void (MastodonAccount*)> callback) {
    QUrl url = QUrl(app->baseUrl);
    url.setPath("/api/v1/accounts/verify_credentials");
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", ("Bearer " + accessToken).toLocal8Bit());

    QNetworkReply *reply = networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [=]()
            {
                // TODO: handle error
                QString err = reply->errorString();
                QByteArray buffer = reply->readAll();
                QJsonDocument jsonDoc((QJsonDocument::fromJson(buffer)));
                QJsonObject jsonReply = jsonDoc.object();
                qDebug() << "json:" << jsonReply;
                MastodonAccount *account = new MastodonAccount(jsonReply, this);
                callback(account);
            });
}
