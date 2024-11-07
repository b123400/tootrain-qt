#ifndef MISSKEYSETTINGWINDOW_H
#define MISSKEYSETTINGWINDOW_H

#include <QDialog>
#include "./misskeyaccount.h"
#include <QListWidgetItem>

namespace Ui {
class MisskeySettingWindow;
}

class MisskeySettingWindow : public QDialog
{
    Q_OBJECT

public:
    explicit MisskeySettingWindow(MisskeyAccount *account, QWidget *parent = nullptr);
    ~MisskeySettingWindow();

public slots:
    void acceptClicked();
    void listItemChanged(QListWidgetItem *item);

signals:
    void accountUpdated(MisskeyAccount *account);

private:
    Ui::MisskeySettingWindow *ui;
    MisskeyAccount *account = nullptr;
    QList<MisskeyStreamSource *>allSources;

    void reloadListItems();
};

#endif // MISSKEYSETTINGWINDOW_H
