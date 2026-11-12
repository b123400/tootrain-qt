#ifndef TOOTRAINENTERPRISEAUTHWINDOW_H
#define TOOTRAINENTERPRISEAUTHWINDOW_H

#include <QDialog>
#include <QLabel>
#include <QUrl>

#include "tootrainslackaccount.h"

// Links a Slack workspace by letting the user through the server's own Slack
// log-in, in their browser.
//
// The server does the OAuth and redirects to
// `tootrain://auth?id=…&slack_access_token=…&token=…`, which comes back here by
// way of `TootRainUrlScheme`. The `token` is the app session: an existing one
// is passed up in the request so the server reuses it and hands the same one
// back, and when none is sent the server makes a new session. `id` is the
// linkage's, which is what the app needs to give the account up again later.
//
// A log-in that did not work comes back the same way, as
// `tootrain://auth?error=<code>&message=<text>`. All of those are the person's
// to read except `invalid_app_session`, which says the session the app sent up
// is not one the server has: that one the app answers itself by throwing the
// session away and starting over, which makes a new one.
//
// Nothing connects until the workspace's users and emoji have been fetched.
class TootRainEnterpriseAuthWindow : public QDialog
{
    Q_OBJECT
public:
    // `teamId` is the workspace to authorise again, when this is a second
    // visit for one the server has let go. Slack is asked for that team by
    // name, so the user is not left picking the right workspace out of the
    // ones they are in.
    explicit TootRainEnterpriseAuthWindow(const QString &teamId = QString(), QWidget *parent = nullptr);

    void reject() override;

signals:
    // The newly linked workspace, which belongs to this window.
    void authenticated(TootRainSlackAccount *account);
    void failed(QString error);

private:
    QUrl logInUrl() const;
    void handleCallback(const QUrl &url);
    void handleFailure(const QString &code, const QString &message);
    void link(const QString &slackAccessToken, const QString &sessionToken, int linkageId);
    void finish(TootRainSlackAccount *account, const QString &error);

    QString teamId;
    QLabel *statusLabel = nullptr;
    // The log-in has come back, so nothing more is taken from the browser, and
    // the window stays until the workspace has been read.
    bool hasFinished = false;
    bool isLinking = false;
    // So a server that keeps refusing the session cannot put the log-in into a
    // loop: the second attempt carries no session at all.
    bool hasStartedOver = false;
};

#endif // TOOTRAINENTERPRISEAUTHWINDOW_H
