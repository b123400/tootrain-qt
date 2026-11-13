#include "tootrainenterpriseclient.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QMessageBox>
#include <QTimer>

#include "slackinfocache.h"
#include "tootrainchannelsubscriptions.h"
#include "tootrainserver.h"
#include "tootrainslackaccount.h"

TootRainEnterpriseClient::TootRainEnterpriseClient(QObject *parent)
    : QObject{parent}
    , centrifuge("TootRain-qt", QCoreApplication::applicationVersion())
{
    // The message schema is not turned into rain drops yet, so for now
    // everything that arrives is logged, as the Mac app does.
    connect(&centrifuge, &CentrifugoClient::connected, this, [](QString clientId) {
        qDebug() << "TootRain: connected as" << clientId;
    });
    connect(&centrifuge, &CentrifugoClient::connecting, this, [](int code, QString reason) {
        qDebug() << "TootRain: connecting" << code << reason;
    });
    connect(&centrifuge, &CentrifugoClient::disconnected, this, &TootRainEnterpriseClient::onDisconnected);
    connect(&centrifuge, &CentrifugoClient::subscribed, this, [](QString channel) {
        qDebug() << "TootRain: listening to" << channel;
    });
    connect(&centrifuge, &CentrifugoClient::unsubscribed, this, [](QString channel) {
        qDebug() << "TootRain: stopped listening to" << channel;
    });
    connect(&centrifuge, &CentrifugoClient::publication, this, [](QString channel, QJsonObject data) {
        qDebug().noquote() << "TootRain:" << channel << "published" << QJsonDocument(data).toJson(QJsonDocument::Compact);
    });
}

void TootRainEnterpriseClient::applicationDidFinishLaunching() {
    refreshSessionInBackground();
    refreshSlackInfoInBackground();
    scheduleConnectionUpdate();
}

// Checks the saved session against the server's own view of it. What comes
// back says which Slack accounts the session still holds, and the accounts it
// does not name are the ones an administrator has logged out: they are marked
// as needing authentication.
//
// A session that cannot be read at all is treated as one that is gone,
// whatever the reason: the server refusing the token, the server being
// unreachable, an answer that cannot be read. Every workspace is then marked
// as needing authentication and the token is dropped, so the app never carries
// on as if a session it cannot see is still there.
void TootRainEnterpriseClient::refreshSessionInBackground() {
    QObject owner;
    if (TootRainSlackAccount::allAccounts(&owner).isEmpty()) return;
    if (TootRainServer::sessionToken().isEmpty()) {
        // Workspaces with no session to reach the server by. Whatever became
        // of the token, they can only be authorised again.
        TootRainSlackAccount::markAllNeedingAuthentication();
        return;
    }
    TootRainServerAPI::shared().session([](QJsonObject overview, QString error) {
        if (!error.isEmpty()) {
            qDebug() << "TootRain: could not ask the server about this session, every workspace needs authorising again:" << error;
            TootRainSlackAccount::forgetSession();
            return;
        }
        qDebug() << "TootRain: the server holds" << overview.value("slack")["linkages"].toArray().size() << "Slack account(s) on this session";
        TootRainSlackAccount::reconcile(overview);
        // Now that every workspace is named by a linkage id and has the
        // channels the server holds for it, the app can say what it means to
        // be listening to, which is not the same thing, since a workspace
        // switched off is in the first and not in the second.
        TootRainChannelSubscriptions::shared().scheduleSync();
    });
}

void TootRainEnterpriseClient::refreshSlackInfoInBackground() {
    QObject owner;
    for (TootRainSlackAccount *account : TootRainSlackAccount::allAccounts(&owner)) {
        QString teamName = account->teamName;
        SlackInfoCache::shared().refresh(account->teamId, account->accessToken, [=](QString error) {
            if (!error.isEmpty()) qDebug() << "TootRain: could not refresh Slack info for" << teamName << error;
        });
    }
}

bool TootRainEnterpriseClient::isAccountUsingTootRainServer(Account *account) {
    return qobject_cast<TootRainSlackAccount *>(account) != nullptr;
}

bool TootRainEnterpriseClient::accountNeedsAuthentication(Account *account) {
    auto slackAccount = qobject_cast<TootRainSlackAccount *>(account);
    return slackAccount && slackAccount->needsAuthentication;
}

void TootRainEnterpriseClient::startStreaming(Account *account) {
    if (!isAccountUsingTootRainServer(account)) return;
    refusedToken.clear();
    TootRainChannelSubscriptions::shared().scheduleSync();
    scheduleConnectionUpdate();
}

void TootRainEnterpriseClient::stopStreaming(Account *account) {
    if (!isAccountUsingTootRainServer(account)) return;
    TootRainChannelSubscriptions::shared().scheduleSync();
    scheduleConnectionUpdate();
}

void TootRainEnterpriseClient::accountStreamingStateChanged(Account *account) {
    if (!isAccountUsingTootRainServer(account)) return;
    TootRainChannelSubscriptions::shared().scheduleSync(true);
}

void TootRainEnterpriseClient::scheduleConnectionUpdate() {
    if (hasScheduledConnectionUpdate) return;
    hasScheduledConnectionUpdate = true;
    QTimer::singleShot(0, this, [this]() {
        hasScheduledConnectionUpdate = false;
        updateConnection();
    });
}

void TootRainEnterpriseClient::updateConnection() {
    QString token = TootRainServer::sessionToken();
    // A connection is wanted for a workspace the server still knows about,
    // and switched on. With every workspace off there is nothing in the
    // session's set for the server to send, so there is nothing for a socket
    // to carry either.
    QObject owner;
    if (token.isEmpty() || TootRainChannelSubscriptions::listeningAccounts(&owner).isEmpty()) {
        disconnectFromServer();
        return;
    }
    if (token == refusedToken) {
        qDebug() << "TootRain: not connecting, the server turned this session away for good";
        return;
    }
    if (isOpen && connectedToken == token) return;

    // No Centrifugo token: the app session token in the header is what the
    // server's connect proxy authenticates, and it answers with the channels
    // this session may hear.
    QUrl endpoint = TootRainServer::centrifugoWebSocketUrl();
    qDebug() << "TootRain: connecting to" << endpoint.toString();
    isOpen = true;
    connectedToken = token;
    centrifuge.connectToServer(endpoint, {
        {"Authorization", "Bearer " + token.toUtf8()},
        {"User-Agent", TootRainServer::userAgent().toUtf8()},
    });
}

void TootRainEnterpriseClient::disconnectFromServer() {
    if (!isOpen) return;
    qDebug() << "TootRain: disconnecting from the TootRain server";
    centrifuge.disconnectFromServer();
    isOpen = false;
    connectedToken.clear();
}

// The server will not take this connection back, which makes this the last
// the app hears of it.
void TootRainEnterpriseClient::onDisconnected(int code, const QString &reason) {
    isOpen = false;
    refusedToken = connectedToken;
    connectedToken.clear();
    qDebug() << "TootRain: the server will not take this connection back" << code << reason;

    // An invalid token, permission denied, and the server's own code for a
    // session it does not know: logging in again is the only way back. Any
    // other is terminal for a reason logging in again would not change, a
    // limit, or a server that is not serving right now.
    if (code != 3500 && code != 3507 && code != 4501) return;
    // The token is worth nothing now, so it goes: the next log-in makes a new
    // session instead of offering this one again.
    TootRainServer::setSessionToken(QString());
    refusedToken.clear();
    // The one terminal disconnect the user can do something about. Nothing
    // else in the app would say so: the rain would simply stop.
    QMessageBox::warning(nullptr, "TootRain",
                         tr("TootRain stopped listening to Slack.") + "\n\n"
                         + tr("The TootRain server no longer knows this TootRain session. Please log in with Slack again."));
}
