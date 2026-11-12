#ifndef SLACKCHANNELSELECTIONWINDOW_H
#define SLACKCHANNELSELECTIONWINDOW_H

#include <QDialog>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QProgressBar>
#include <QPushButton>
#include <QSet>

#include "tootrainslackaccount.h"

// Picks which channels of one workspace the app listens to.
//
// The channels come from Slack (`conversations.list`), and only the ones the
// user is in are offered: the TootRain server grants a channel on Slack's word
// that they are a member of it, so a tick on anything else could only be
// refused.
//
// Nothing is sent while the ticking is going on. Done states the whole set in
// one request (the server holds one set per session, so that request carries
// the other workspaces' channels as well) and Cancel closes the picker having
// said nothing, which is all it takes to put the ticks back. Both buttons are
// out of reach while a request is in flight: there is nothing to say Done to
// before the list arrives, and nothing to take back until the set has either
// been accepted or refused.
class SlackChannelSelectionWindow : public QDialog
{
    Q_OBJECT
public:
    explicit SlackChannelSelectionWindow(TootRainSlackAccount *account, QWidget *parent = nullptr);

    void reject() override;

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    struct Row {
        QString channelId;
        // A channel the user has picked that Slack did not list (one they
        // have since left, say) is named by what the server last knew it as,
        // or by its id when nothing knows a name, so that it can be unticked.
        QString name;
        bool isPrivate;
        bool isListedBySlack;

        QString title() const { return (isPrivate ? "🔒 " : "# ") + name; }
    };

    void reload();
    void present(const QJsonArray &channels, const QVariantMap &picked, bool isFinal);
    void presentLoadingFailure(const QString &error);
    void applyFilter();
    void updateStatusLabel();
    void setBusy(bool busy, const QString &status = QString());
    void itemChanged(QListWidgetItem *item);
    void donePressed();
    void finishSaving(const QJsonObject &update);

    // What is saved for the workspace, which belongs to this window.
    TootRainSlackAccount *account;

    // Every channel on offer; the list shows the ones the filter leaves.
    QList<Row> allRows;
    // What the ticks say now, and what they said when the picker opened. Done
    // compares the two, so a picker closed without a change says nothing to
    // the server at all.
    QSet<QString> selectedChannelIds;
    QSet<QString> openedWithChannelIds;
    // A request is in flight: the list on the way in, or the set on the way
    // out.
    bool isBusy = false;

    QLabel *statusLabel;
    QProgressBar *loadingIndicator;
    QLineEdit *filterField;
    QListWidget *listWidget;
    QPushButton *doneButton;
    QPushButton *cancelButton;
};

#endif // SLACKCHANNELSELECTIONWINDOW_H
