#include "tootrainenterpriseauthwindow.h"

#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QPointer>
#include <QUrlQuery>
#include <QVBoxLayout>

#include "slackapiclient.h"
#include "slackinfocache.h"
#include "tootrainserver.h"
#include "tootrainurlscheme.h"

TootRainEnterpriseAuthWindow::TootRainEnterpriseAuthWindow(const QString &teamId, QWidget *parent)
    : QDialog(parent)
    , teamId(teamId)
{
    setWindowTitle(tr("Log in with Slack"));
    statusLabel = new QLabel(tr("Please log in with Slack in your browser."), this);
    auto buttons = new QDialogButtonBox(QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &TootRainEnterpriseAuthWindow::reject);
    auto layout = new QVBoxLayout(this);
    layout->addWidget(statusLabel);
    layout->addWidget(buttons);

    // Only while this window is open is a link taken at all: any page can
    // open a tootrain:// link, and one that arrives unasked is not a log-in.
    TootRainUrlScheme::shared().listen();
    connect(&TootRainUrlScheme::shared(), &TootRainUrlScheme::urlReceived, this, &TootRainEnterpriseAuthWindow::handleCallback);
    QDesktopServices::openUrl(logInUrl());
}

void TootRainEnterpriseAuthWindow::reject() {
    if (!isLinking) QDialog::reject();
}

QUrl TootRainEnterpriseAuthWindow::logInUrl() const {
    QUrlQuery query;
    // Percent-encoded, so a `+` in the token is not read as a space.
    QString token = TootRainServer::sessionToken();
    if (!token.isEmpty()) query.addQueryItem("token", QString::fromLatin1(QUrl::toPercentEncoding(token)));
    if (!teamId.isEmpty()) query.addQueryItem("team", QString::fromLatin1(QUrl::toPercentEncoding(teamId)));
    QUrl url = TootRainServer::logInWithSlackUrl();
    url.setQuery(query);
    return url;
}

void TootRainEnterpriseAuthWindow::handleCallback(const QUrl &url) {
    if (hasFinished || url.scheme() != "tootrain" || url.host() != "auth") return;
    raise();
    activateWindow();
    QUrlQuery query(url);
    auto value = [&](const QString &name) {
        return query.queryItemValue(name, QUrl::FullyDecoded);
    };
    if (!value("error").isEmpty()) {
        handleFailure(value("error"), value("message"));
        return;
    }
    QString slackAccessToken = value("slack_access_token");
    QString sessionToken = value("token");
    if (slackAccessToken.isEmpty() || sessionToken.isEmpty()) {
        finish(nullptr, tr("The TootRain server did not send back a Slack token."));
        return;
    }
    // The linkage id is what lets the app give this account up again. An
    // answer without one still links the account; it just has to be asked
    // for again, by the session overview at the next launch.
    int linkageId = value("id").toInt();
    if (!linkageId) qDebug() << "TootRain: the log-in sent back no linkage id";
    link(slackAccessToken, sessionToken, linkageId);
}

// The log-in did not work. Everything but a session the server cannot place
// is the person's to read; that one the app answers itself, once, by starting
// the log-in over with no session at all.
void TootRainEnterpriseAuthWindow::handleFailure(const QString &code, const QString &message) {
    if (code != "invalid_app_session" || hasStartedOver || TootRainServer::sessionToken().isEmpty()) {
        finish(nullptr, message.isEmpty() ? tr("The TootRain server refused the log-in: %1").arg(code) : message);
        return;
    }
    hasStartedOver = true;
    qDebug() << "TootRain: the server does not know our app session, starting a new one";
    TootRainServer::setSessionToken(QString());
    QDesktopServices::openUrl(logInUrl());
}

// Saves both tokens, works out which workspace was authorised, and fills the
// caches the renderer will need before anything connects.
void TootRainEnterpriseAuthWindow::link(const QString &slackAccessToken, const QString &sessionToken, int linkageId) {
    hasFinished = true;
    isLinking = true;
    statusLabel->setText(tr("Loading channels…"));
    QPointer<TootRainEnterpriseAuthWindow> self(this);
    SlackAPIClient::shared().identity(slackAccessToken, [=](QJsonObject identity, QString error) {
        if (!self) return;
        if (!error.isEmpty()) {
            finish(nullptr, error);
            return;
        }
        TootRainServer::setSessionToken(sessionToken);

        QString teamId = identity.value("team_id").toString();
        auto account = new TootRainSlackAccount(teamId,
                                                identity.value("team").toString(teamId),
                                                QUrl(identity.value("url").toString()).host(),
                                                identity.value("user_id").toString(),
                                                identity.value("user").toString(),
                                                this);
        account->linkageId = linkageId;
        account->accessToken = slackAccessToken;
        // Saving a fresh row is also what clears any needs-authentication mark
        // this workspace was carrying, which is the point of logging in again.
        account->save();
        TootRainSlackAccount::notifyAccountsChanged();

        SlackInfoCache::shared().refresh(teamId, slackAccessToken, [=](QString error) {
            if (!self) return;
            finish(error.isEmpty() ? account : nullptr, error);
        });
    });
}

void TootRainEnterpriseAuthWindow::finish(TootRainSlackAccount *account, const QString &error) {
    hasFinished = true;
    isLinking = false;
    // Closed before anything is said, so what the caller shows next is not
    // behind this window. The window itself outlives the call.
    done(account ? QDialog::Accepted : QDialog::Rejected);
    if (account) {
        emit authenticated(account);
    } else {
        emit failed(error);
    }
}
