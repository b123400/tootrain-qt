#include "slackchannelselectionwindow.h"

#include <QCollator>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QMessageBox>
#include <QPointer>
#include <QVBoxLayout>
#include <algorithm>

#include "slackapiclient.h"
#include "slackinfocache.h"
#include "tootrainchannelsubscriptions.h"
#include "tootrainserver.h"

SlackChannelSelectionWindow::SlackChannelSelectionWindow(TootRainSlackAccount *account, QWidget *parent)
    : QDialog(parent)
    , account(account)
{
    account->setParent(this);
    setWindowTitle(tr("Channels of %1").arg(account->teamName));
    resize(460, 470);

    statusLabel = new QLabel(this);
    // The spinner is in the far corner so that the status text starts in the
    // same place whether or not anything is loading.
    loadingIndicator = new QProgressBar(this);
    loadingIndicator->setRange(0, 0);
    loadingIndicator->setTextVisible(false);
    loadingIndicator->setFixedWidth(60);
    auto statusRow = new QHBoxLayout();
    statusRow->addWidget(statusLabel, 1);
    statusRow->addWidget(loadingIndicator);

    filterField = new QLineEdit(this);
    filterField->setPlaceholderText(tr("Filter channels"));
    filterField->setClearButtonEnabled(true);
    connect(filterField, &QLineEdit::textChanged, this, &SlackChannelSelectionWindow::applyFilter);

    listWidget = new QListWidget(this);
    listWidget->setSelectionMode(QAbstractItemView::NoSelection);
    connect(listWidget, &QListWidget::itemChanged, this, &SlackChannelSelectionWindow::itemChanged);

    cancelButton = new QPushButton(tr("Cancel"), this);
    cancelButton->setAutoDefault(false);
    connect(cancelButton, &QPushButton::clicked, this, &SlackChannelSelectionWindow::reject);
    // Return sends the set.
    doneButton = new QPushButton(tr("Done"), this);
    doneButton->setDefault(true);
    connect(doneButton, &QPushButton::clicked, this, &SlackChannelSelectionWindow::donePressed);
    auto buttonRow = new QHBoxLayout();
    buttonRow->addStretch(1);
    buttonRow->addWidget(cancelButton);
    buttonRow->addWidget(doneButton);

    auto layout = new QVBoxLayout(this);
    layout->addLayout(statusRow);
    layout->addWidget(filterField);
    layout->addWidget(listWidget, 1);
    layout->addLayout(buttonRow);

    reload();
}

void SlackChannelSelectionWindow::reload() {
    bool isListening = TootRainChannelSubscriptions::isListening(account);
    QVariantMap remembered = account->channels.value_or(QVariantMap());
    QString teamId = account->teamId;

    // What the last fetch left cached goes up at once, so there is a list to
    // read and filter while Slack is asked for it again. The ticks on it are
    // the channels the app remembers picking, which is a good guess and not an
    // answer: what a workspace that is switched on is listening to is the
    // server's to say, and until it has said, there is nothing to press Done
    // on either.
    QJsonArray cached = SlackInfoCache::shared().channels(teamId);
    if (cached.isEmpty()) {
        setBusy(true, tr("Loading channels…"));
    } else {
        setBusy(true, tr("Updating channels…"));
        present(cached, remembered, false);
    }

    QPointer<SlackChannelSelectionWindow> self(this);
    SlackAPIClient::shared().conversations(account->accessToken, [=](QJsonArray channels, QString error) {
        if (!self) return;
        if (!error.isEmpty()) {
            presentLoadingFailure(error);
            return;
        }
        // Slack has just said what this workspace's channels are, so the next
        // picker, and the next message with a channel in it, can open on that
        // rather than on what the last launch saw.
        SlackInfoCache::shared().updateChannels(channels, teamId);

        // What the ticks start as. For a workspace that is switched on it is
        // what the server actually holds, which can be less than the app has
        // written down: a channel the user has left is dropped at the
        // server's end. For one that is switched off the server holds nothing
        // of it at all, so what the app remembers is the only answer there is.
        if (!isListening) {
            present(channels, remembered, true);
            return;
        }
        TootRainServerAPI::shared().channelMemberships(teamId, [=](QJsonObject answer, QString error) {
            if (!self) return;
            if (!error.isEmpty()) {
                presentLoadingFailure(error);
                return;
            }
            present(channels, TootRainServerAPI::channelNames(answer.value("memberships").toArray()), true);
        });
    });
}

// Puts a list of channels on screen, with `picked` ticked. `isFinal` says this
// one is Slack's answer rather than the cached list shown while waiting for
// it: only then do the ticks say what the server holds, which is what Done
// compares against before it sends anything, and only then is the picker ready
// to be used at all.
void SlackChannelSelectionWindow::present(const QJsonArray &channels, const QVariantMap &picked, bool isFinal) {
    const QStringList pickedIds = picked.keys();
    selectedChannelIds = QSet<QString>(pickedIds.begin(), pickedIds.end());
    if (isFinal) {
        openedWithChannelIds = selectedChannelIds;
    }

    QList<Row> rows;
    QSet<QString> listedIds;
    for (const QJsonValue &channel : channels) {
        if (!channel["isMember"].toBool()) continue;
        rows.append(Row{channel["id"].toString(), channel["name"].toString(), channel["isPrivate"].toBool(), true});
        listedIds.insert(channel["id"].toString());
    }
    QList<Row> unlisted;
    for (const QString &channelId : pickedIds) {
        if (listedIds.contains(channelId)) continue;
        unlisted.append(Row{channelId, picked[channelId].toString(), false, false});
    }
    // The way a person would order them: "channel2" before "channel10".
    QCollator collator;
    collator.setNumericMode(true);
    collator.setCaseSensitivity(Qt::CaseInsensitive);
    auto byName = [&](const Row &a, const Row &b) { return collator.compare(a.name, b.name) < 0; };
    std::sort(rows.begin(), rows.end(), byName);
    std::sort(unlisted.begin(), unlisted.end(), byName);

    allRows = rows + unlisted;
    if (isFinal) {
        setBusy(false);
    }
    applyFilter();
}

// There is nothing to pick without the list, so a request that fails on the
// way to it closes the picker and says why, on the window the picker was
// opened from.
void SlackChannelSelectionWindow::presentLoadingFailure(const QString &error) {
    QWidget *parent = parentWidget();
    isBusy = false;
    reject();
    QMessageBox::warning(parent, "TootRain", error);
}

// The list shows what the filter leaves; the count keeps speaking of every
// channel, since that is what the ticks are about.
void SlackChannelSelectionWindow::applyFilter() {
    QString query = filterField->text().trimmed();
    QSignalBlocker blocker(listWidget);
    listWidget->clear();
    for (const Row &row : allRows) {
        if (!query.isEmpty() && !row.name.contains(query, Qt::CaseInsensitive)) continue;
        bool isPicked = selectedChannelIds.contains(row.channelId);
        auto item = new QListWidgetItem(row.title(), listWidget);
        item->setData(Qt::UserRole, row.channelId);
        item->setCheckState(isPicked ? Qt::Checked : Qt::Unchecked);
        // A channel Slack did not list can be unticked but not ticked again,
        // there being no membership to grant it on.
        bool isEnabled = !isBusy && (row.isListedBySlack || isPicked);
        item->setFlags(isEnabled ? Qt::ItemIsEnabled | Qt::ItemIsUserCheckable : Qt::NoItemFlags);
    }
    updateStatusLabel();
}

// A request in flight has the status line to itself.
void SlackChannelSelectionWindow::updateStatusLabel() {
    if (isBusy) return;
    if (listWidget->count() == 0 && !filterField->text().trimmed().isEmpty()) {
        statusLabel->setText(tr("No channel matches this filter."));
        return;
    }
    statusLabel->setText(tr("Checked channels are shown as comments. %1 of %2 selected.")
                             .arg(selectedChannelIds.size()).arg(allRows.size()));
}

// Both buttons and every tick are out of reach while a request is in flight,
// and the spinner in the corner is what says so.
void SlackChannelSelectionWindow::setBusy(bool busy, const QString &status) {
    isBusy = busy;
    if (!status.isEmpty()) {
        statusLabel->setText(status);
    }
    doneButton->setEnabled(!busy);
    cancelButton->setEnabled(!busy);
    loadingIndicator->setVisible(busy);
    applyFilter();
}

void SlackChannelSelectionWindow::itemChanged(QListWidgetItem *item) {
    QString channelId = item->data(Qt::UserRole).toString();
    if (item->checkState() == Qt::Checked) {
        selectedChannelIds.insert(channelId);
    } else {
        selectedChannelIds.remove(channelId);
    }
    updateStatusLabel();
}

// Says what this session listens to, with this workspace's channels as the
// ticks now have them. One request, whatever was ticked and unticked, and the
// picker stays where it is until the server has answered.
void SlackChannelSelectionWindow::donePressed() {
    if (isBusy) return;
    QSet<QString> wanted = selectedChannelIds;
    if (wanted == openedWithChannelIds) {
        accept();
        return;
    }
    // The names go up with the ids, so what the user picked can be named
    // again without asking Slack.
    QVariantMap picks;
    for (const Row &row : allRows) {
        if (wanted.contains(row.channelId)) picks[row.channelId] = row.name;
    }
    setBusy(true, tr("Saving channels…"));
    QPointer<SlackChannelSelectionWindow> self(this);
    TootRainChannelSubscriptions::shared().apply(picks, account, [=](QJsonObject update, QString error) {
        if (!self) return;
        if (error.isEmpty()) {
            finishSaving(update);
            return;
        }
        // The whole set failed rather than one channel of it, so nothing has
        // changed and the picker stays open to be tried again.
        setBusy(false);
        QMessageBox::warning(this, "TootRain", error);
    });
}

void SlackChannelSelectionWindow::finishSaving(const QJsonObject &update) {
    // The linkage id as it is saved now: the statement may have been what
    // learned it.
    QObject owner;
    TootRainSlackAccount *saved = TootRainSlackAccount::accountWithTeamId(account->teamId, &owner);
    int linkageId = saved ? saved->linkageId : 0;
    // A workspace that is switched off was not in the request at all, its
    // channels are only written down, so there the ticks are what was picked.
    if (update.isEmpty() || !linkageId) {
        accept();
        return;
    }
    // What the server holds afterwards is what the ticks are: it may be less
    // than was asked for.
    const QStringList held = TootRainServerAPI::channelNames(
        TootRainServerAPI::ofLinkage(update["memberships"].toArray(), linkageId)).keys();
    selectedChannelIds = QSet<QString>(held.begin(), held.end());
    openedWithChannelIds = selectedChannelIds;
    const QJsonArray rejected = TootRainServerAPI::ofLinkage(update["rejected"].toArray(), linkageId);
    if (rejected.isEmpty()) {
        accept();
        return;
    }
    // A channel the server would not grant does not take the rest of the set
    // with it: the others are being listened to, and the ticks have already
    // gone back to what the server actually holds. So the picker stays where
    // it is and says which ones did not happen.
    setBusy(false);
    QStringList reasons;
    for (const QJsonValue &rejection : rejected) {
        QString title = rejection["channel_id"].toString();
        for (const Row &row : allRows) {
            if (row.channelId == title) title = row.title();
        }
        reasons.append(tr("%1: %2").arg(title, TootRainServerAPI::errorNamed(rejection["error"].toString(), rejection["detail"].toString())));
    }
    QMessageBox::warning(this, "TootRain", tr("Some channels could not be added.") + "\n\n" + reasons.join("\n"));
}

// Closing is Cancel, and like Cancel it waits for a request in flight.
void SlackChannelSelectionWindow::reject() {
    if (!isBusy) QDialog::reject();
}

// Esc empties the filter before it closes the picker, the way a search field's
// own Esc would, so one keystroke never throws away both what was typed and
// the window it was typed in.
void SlackChannelSelectionWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape && !filterField->text().isEmpty()) {
        if (!isBusy) filterField->clear();
        return;
    }
    QDialog::keyPressEvent(event);
}
