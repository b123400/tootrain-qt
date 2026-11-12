#include "tootrainchannelsubscriptions.h"

#include <QApplication>
#include <QJsonArray>
#include <QMessageBox>
#include <QTimer>

#include "../streammanager.h"
#include "tootrainserver.h"

void TootRainChannelSubscriptions::scheduleSync(bool askedFor) {
    scheduledSyncWasAskedFor = scheduledSyncWasAskedFor || askedFor;
    if (hasScheduledSync) return;
    hasScheduledSync = true;
    QTimer::singleShot(0, this, [this]() {
        hasScheduledSync = false;
        bool askedFor = scheduledSyncWasAskedFor;
        scheduledSyncWasAskedFor = false;
        state(QString(), QVariantMap(), [=](QJsonObject, QString error) {
            if (error.isEmpty()) return;
            qDebug() << "TootRain: could not tell the server what to listen to:" << error;
            // What the user asked for is still written down and goes up with
            // the next statement, so this says what did not happen rather
            // than undoing it.
            if (askedFor) {
                QMessageBox::warning(QApplication::activeWindow(), "TootRain",
                                     tr("TootRain could not tell the server which channels to listen to.") + "\n\n" + error);
            }
        });
    });
}

void TootRainChannelSubscriptions::apply(const QVariantMap &channels, TootRainSlackAccount *account, Callback callback) {
    if (isListening(account)) {
        state(account->teamId, channels, callback);
        return;
    }
    // Written to the account as it is saved now, rather than as the caller
    // was handed it.
    QObject owner;
    TootRainSlackAccount *saved = TootRainSlackAccount::accountWithTeamId(account->teamId, &owner);
    if (saved) {
        saved->channels = channels;
        saved->save();
    }
    emit didChange();
    callback(QJsonObject(), QString());
}

bool TootRainChannelSubscriptions::isListening(Account *account) {
    return StreamManager::shared().isAccountStreaming(account);
}

QList<TootRainSlackAccount *> TootRainChannelSubscriptions::listeningAccounts(QObject *owner) {
    QList<TootRainSlackAccount *> result;
    for (TootRainSlackAccount *account : TootRainSlackAccount::allAccounts(owner)) {
        if (!account->needsAuthentication && isListening(account)) result.append(account);
    }
    return result;
}

// The statement itself, with "this is in flight" either side of it. Asking the
// server what the session is counts, since that is part of the same wait.
void TootRainChannelSubscriptions::state(const QString &changedTeamId, const QVariantMap &changedChannels, Callback callback) {
    statementsInFlight++;
    emit didChange();
    Callback ended = [=](QJsonObject update, QString error) {
        statementsInFlight = qMax(0, statementsInFlight - 1);
        emit didChange();
        callback(update, error);
    };

    // The set names a Slack account by the server's own linkage id, which is
    // the one id here the app is not told until it asks what its session is.
    // A launch that states the set before that answer arrives asks for it
    // first rather than leaving an account out. The same answer is where a
    // workspace with no set of its own yet gets one, which has to happen
    // before the whole set is stated or the statement would give up the very
    // channels it has not learned about.
    QObject owner;
    bool knowsEveryAccount = true;
    for (TootRainSlackAccount *account : listeningAccounts(&owner)) {
        if (!account->linkageId || !account->channels) knowsEveryAccount = false;
    }
    if (knowsEveryAccount) {
        stateWholeSet(changedTeamId, changedChannels, ended);
        return;
    }
    TootRainServerAPI::shared().session([=](QJsonObject overview, QString error) {
        if (!error.isEmpty()) {
            ended(QJsonObject(), error);
            return;
        }
        // The answer also says which accounts the server no longer holds, so
        // an account left out of the set afterwards is one there is nothing
        // to be said about.
        TootRainSlackAccount::reconcile(overview);
        stateWholeSet(changedTeamId, changedChannels, ended);
    });
}

void TootRainChannelSubscriptions::stateWholeSet(const QString &changedTeamId, const QVariantMap &changedChannels, Callback callback) {
    QObject owner;
    QJsonArray pairs;
    QStringList named;
    for (TootRainSlackAccount *account : listeningAccounts(&owner)) {
        if (!account->linkageId) {
            qDebug() << "TootRain: the server has no linkage for" << account->teamId << "leaving its channels out";
            continue;
        }
        named.append(account->teamId);
        QVariantMap channels = account->teamId == changedTeamId ? changedChannels : account->channels.value_or(QVariantMap());
        for (const QString &channelId : channels.keys()) {
            pairs.append(QJsonObject{{"linkage_id", account->linkageId}, {"channel_id", channelId}});
        }
    }
    // Stating the set without the workspace the picks belong to would take its
    // channels away rather than add them, so that is not sent at all.
    if (!changedTeamId.isEmpty() && !named.contains(changedTeamId)) {
        callback(QJsonObject(), TootRainServerAPI::errorNamed("not_linked"));
        return;
    }
    TootRainServerAPI::shared().replaceChannelMemberships(pairs, [=](QJsonObject update, QString error) {
        if (error.isEmpty()) commit(update, named);
        callback(update, error);
    });
}

// Takes the server's answer as what each workspace of the statement holds. It
// is the part to settle on: a channel the bot was never invited to is not in
// it, however firmly the app asked for it.
void TootRainChannelSubscriptions::commit(const QJsonObject &update, const QStringList &teamIds) {
    QObject owner;
    for (TootRainSlackAccount *account : TootRainSlackAccount::allAccounts(&owner)) {
        if (!teamIds.contains(account->teamId) || !account->linkageId) continue;
        account->channels = TootRainServerAPI::channelNames(
            TootRainServerAPI::ofLinkage(update["memberships"].toArray(), account->linkageId));
        account->save();
    }
}
