#ifndef MISSKEYSETTINGWINDOW_H
#define MISSKEYSETTINGWINDOW_H

#include <QDialog>
#include "./misskeyaccount.h"

namespace Ui {
class MisskeySettingWindow;
}

class MisskeySettingWindow : public QDialog
{
    Q_OBJECT

public:
    explicit MisskeySettingWindow(QWidget *parent = nullptr);
    ~MisskeySettingWindow();

public slots:
    void acceptClicked();
    void channelButtonClicked();

signals:
    void accountUpdated(MisskeyAccount *account);

private:
    Ui::MisskeySettingWindow *ui;
};

#endif // MISSKEYSETTINGWINDOW_H
