#include "misskeyclient.h"
#include <QNetworkReply>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

using namespace Qt::Literals::StringLiterals;

MisskeyClient::MisskeyClient(QObject *parent)
    : QObject{parent}
{
    networkManager = new QNetworkAccessManager(this);
}

void MisskeyClient::fetchWellKnown(QUrl host, std::function<void (MisskeyWellKnown *)> callback) {
    QUrl baseUrl = QUrl(host);
    baseUrl.setPath("");
    QUrl url = QUrl(baseUrl);
    url.setPath("/.well-known/oauth-authorization-server");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QNetworkReply *reply = networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [=]()
            {
                // TODO: handle error
                // QString err = reply->errorString();
                QByteArray buffer = reply->readAll();
                QJsonDocument jsonDoc((QJsonDocument::fromJson(buffer)));
                QJsonObject jsonReply = jsonDoc.object();
                // qDebug() << "json:" << jsonReply;
                callback(new MisskeyWellKnown(jsonReply, this));
            });
}

void MisskeyClient::fetchAccountWithToken(QUrl host, QString token, std::function<void (MisskeyAccount *)> callback) {
    QUrl baseUrl = QUrl(host);
    baseUrl.setPath("");
    QUrl url = QUrl(baseUrl);
    url.setPath("/api/i");
    QNetworkRequest request(url);
    QHttpHeaders headers;
    headers.append(QHttpHeaders::WellKnownHeader::Authorization, u"Bearer "_s + token);
    headers.append(QHttpHeaders::WellKnownHeader::ContentType, "application/json");
    request.setHeaders(headers);
    auto reply = networkManager->post(request, "{}");
    connect(reply, &QNetworkReply::finished, this, [=]()
            {
                // TODO: handle error
                // QString err = reply->errorString();
                QByteArray buffer = reply->readAll();
                QJsonDocument jsonDoc((QJsonDocument::fromJson(buffer)));
                QJsonObject jsonReply = jsonDoc.object();
                // qDebug() << "json:" << jsonReply;
                auto acc = new MisskeyAccount(jsonReply, this);
                callback(acc);
            });
}