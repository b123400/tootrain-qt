#ifndef MISSKEYWELLKNOWN_H
#define MISSKEYWELLKNOWN_H

#include <QObject>
#include <QUrl>
#include <QJsonObject>

class MisskeyWellKnown : public QObject
{
    Q_OBJECT
public:
    MisskeyWellKnown(QJsonObject json, QObject *parent = nullptr);

    QUrl authenticateUrl;
    QUrl accessTokenUrl;

signals:
};

#endif // MISSKEYWELLKNOWN_H
