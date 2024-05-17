#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <QObject>

class Account : public QObject
{
    Q_OBJECT
public:
    explicit Account(QObject *parent = nullptr);

    QString displayName;
    QString username;
    QString id;
    QString avatarUrl;

signals:
};

#endif // ACCOUNT_H
