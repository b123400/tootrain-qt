#ifndef MASTODONSETTINGWINDOW_H
#define MASTODONSETTINGWINDOW_H

#include <QDialog>
#include "account.h"

namespace Ui {
class MastodonSettingWindow;
}

class MastodonSettingWindow : public QDialog
{
    Q_OBJECT

public:
    explicit MastodonSettingWindow(MastodonAccount *account, QWidget *parent = nullptr);
    ~MastodonSettingWindow();

public slots:
    void sourceButtonClicked();
    void okClicked();
    void cancelClicked();
    void hashtagChanged();
    void hashtagLocalChanged();
    void listSelected(int index);

signals:
    void accountUpdated(MastodonAccount *account);

private:
    Ui::MastodonSettingWindow *ui;
    MastodonAccount *account;
    QString currentHashtag;

    void updateUiFromSettings();
    void updateButtonState();

    void updateLists();
};

#endif // MASTODONSETTINGWINDOW_H
