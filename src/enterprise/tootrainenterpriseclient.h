#ifndef TOOTRAINENTERPRISECLIENT_H
#define TOOTRAINENTERPRISECLIENT_H

#include <QJsonObject>
#include <QObject>

#include "../account.h"
#include "centrifugoclient.h"

// The app's one connection to the TootRain server.
//
// There is at most one, however many Slack workspaces are linked, which is
// what keeps the server's load down. It is a Centrifugo connection: the app
// opens the WebSocket with the app session token in the `Authorization`
// header, Centrifugo asks the server who is connecting through its connect
// proxy, and the server answers with the channels this session may hear. The
// app never subscribes to anything itself: the channels come with the
// connection, and the server adds and drops them on a live one as the
// session's channel memberships change.
class TootRainEnterpriseClient : public QObject
{
    Q_OBJECT
public:
    static TootRainEnterpriseClient &shared() {static TootRainEnterpriseClient client; return client;}

    // Asks the server what the saved session still is, brings the workspace
    // caches up to date, and opens the connection if there is a workspace
    // switched on, all in the background, so a launch with nothing linked
    // opens no socket at all.
    void applicationDidFinishLaunching();

    // Whether this account is streamed by the TootRain server rather than by a
    // WebSocket of its own. The one place that knows which kind of account
    // that is, so the rest of the app can ask instead of testing a class.
    static bool isAccountUsingTootRainServer(Account *account);

    // Whether the server has stopped recognising this account, which leaves
    // nothing to be done with it until the user authorises it again.
    static bool accountNeedsAuthentication(Account *account);

    // Where a workspace being switched on arrives, by way of `StreamManager`.
    // It opens no connection of its own, there is only ever the one, but it is
    // the user asking for that one, so a refusal the app was sitting on is
    // worth another attempt. What it does do is put the workspace's channels
    // back into the set this session listens to, which is the whole of what
    // "connected" means here.
    void startStreaming(Account *account);

    // Switching a workspace off takes its channels out of the set. The
    // channels to put back are kept on the account, and the connection itself
    // only closes if nothing is left switched on.
    void stopStreaming(Account *account);

    // The user switched a workspace on or off themselves, rather than the app
    // restoring at launch what they left switched on. The difference is only
    // in who is told when the statement does not reach the server.
    void accountStreamingStateChanged(Account *account);

    // Opens or closes the connection to match what is wanted now: a session
    // token, and a workspace to listen to.
    void updateConnection();

signals:
    // What is saved for the Slack workspaces has changed in a way the settings
    // window shows: one the server no longer has, or has again.
    void slackAccountsChanged();

private:
    explicit TootRainEnterpriseClient(QObject *parent = nullptr);

    void refreshSessionInBackground();
    void refreshSlackInfoInBackground();
    void scheduleConnectionUpdate();
    void disconnectFromServer();
    void onDisconnected(int code, const QString &reason);

    CentrifugoClient centrifuge;
    // Whether the connection is open, or being kept open, and the token it
    // was opened with, so a changed session reconnects instead of keeping a
    // connection the server no longer recognises.
    bool isOpen = false;
    QString connectedToken;
    // Set while a connection update is waiting for the next turn of the event
    // loop, so the run of `startStreaming` calls every launch makes opens one
    // connection rather than one per workspace.
    bool hasScheduledConnectionUpdate = false;
    // A session the server closed the connection on for good, kept so nothing
    // reopens a socket that would only be refused again.
    QString refusedToken;
};

#endif // TOOTRAINENTERPRISECLIENT_H
