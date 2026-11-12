#ifndef TOOTRAINCHANNELSUBSCRIPTIONS_H
#define TOOTRAINCHANNELSUBSCRIPTIONS_H

#include <QJsonObject>
#include <QObject>
#include <QStringList>
#include <QVariantMap>
#include <functional>

#include "tootrainslackaccount.h"

// What this app session is listening to, stated as one set.
//
// The server holds one set per session rather than one per workspace, and the
// app states the whole of it in a single request: `PUT
// /api/slack/channel_memberships` carries every pair of a Slack account and
// one of its channels, and the server works out the difference from what it
// holds. So every change comes through here, which is the one place that knows
// the whole set: a workspace switched on or off, and the channels the picker
// leaves ticked.
//
// Switching a workspace off is that set with its pairs left out. The server
// has no notion of a linkage being off for a while, so it is this app's doing,
// and what to put back when the workspace is switched on again is kept on the
// account rather than on the server.
class TootRainChannelSubscriptions : public QObject
{
    Q_OBJECT
public:
    // The server's answer to the statement, or else why it did not happen.
    // Both are empty when there was nothing to say.
    typedef std::function<void (QJsonObject update, QString error)> Callback;

    static TootRainChannelSubscriptions &shared() {static TootRainChannelSubscriptions subscriptions; return subscriptions;}

    // Whether a statement is in flight, which is what the Connect button shows
    // as a workspace being switched on or off.
    bool isStating() const { return statementsInFlight > 0; }

    // Brings the server's set into line with what the app wants, on the next
    // turn of the event loop, so the run of calls a launch makes states the
    // set once rather than once per workspace.
    //
    // `askedFor` is the user waiting on it: the Connect button, rather than a
    // launch or a session the app refreshed by itself. A statement nobody is
    // waiting on is only logged when it fails: what the app wants is written
    // down either way, and the next statement carries it up.
    void scheduleSync(bool askedFor = false);

    // States the set with one workspace's channels replaced by what the user
    // has just picked. Nothing is saved until the server has taken it, so a
    // request that fails leaves the app where it was.
    //
    // There is nothing to say for a workspace that is switched off, which is
    // not in the set either way: the picks are only written down, to go up
    // with the set when it is switched on again.
    void apply(const QVariantMap &channels, TootRainSlackAccount *account, Callback callback);

    // Whether this workspace is switched on, which is the same flag the
    // Connect button writes. The one place that asks, so both sides of what
    // "connected" means, the set stated to the server and the socket it
    // arrives on, read it the same way.
    static bool isListening(Account *account);

    // The workspaces whose channels belong in the set: linked, and switched on.
    static QList<TootRainSlackAccount *> listeningAccounts(QObject *owner);

signals:
    // A statement has started or ended, or the picked channels have changed.
    void didChange();

private:
    explicit TootRainChannelSubscriptions(QObject *parent = nullptr) : QObject{parent} {}

    void state(const QString &changedTeamId, const QVariantMap &changedChannels, Callback callback);
    void stateWholeSet(const QString &changedTeamId, const QVariantMap &changedChannels, Callback callback);
    void commit(const QJsonObject &update, const QStringList &teamIds);

    bool hasScheduledSync = false;
    // Whether the user asked for the statement that is being waited for. They
    // are owed an answer when it does not reach the server.
    bool scheduledSyncWasAskedFor = false;
    int statementsInFlight = 0;
};

#endif // TOOTRAINCHANNELSUBSCRIPTIONS_H
