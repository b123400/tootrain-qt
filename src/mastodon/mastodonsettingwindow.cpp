#include "mastodonsettingwindow.h"
#include "ui_mastodonsettingwindow.h"
#include "client.h"

MastodonSettingWindow::MastodonSettingWindow(MastodonAccount*account, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MastodonSettingWindow)
{
    this->account = account;
    account->setParent(this);
    ui->setupUi(this);

    connect(ui->publicButton, &QAbstractButton::clicked, this, &MastodonSettingWindow::sourceButtonClicked);
    connect(ui->publicLocalButton, &QAbstractButton::clicked, this, &MastodonSettingWindow::sourceButtonClicked);
    connect(ui->publicRemoteButton, &QAbstractButton::clicked, this, &MastodonSettingWindow::sourceButtonClicked);
    connect(ui->hashtagButton, &QAbstractButton::clicked, this, &MastodonSettingWindow::sourceButtonClicked);
    connect(ui->hashtagLocalButton, &QAbstractButton::clicked, this, &MastodonSettingWindow::sourceButtonClicked);
    connect(ui->userButton, &QAbstractButton::clicked, this, &MastodonSettingWindow::sourceButtonClicked);
    connect(ui->userNotificationButton, &QAbstractButton::clicked, this, &MastodonSettingWindow::sourceButtonClicked);
    connect(ui->listButton, &QAbstractButton::clicked, this, &MastodonSettingWindow::sourceButtonClicked);
    connect(ui->directButton, &QAbstractButton::clicked, this, &MastodonSettingWindow::sourceButtonClicked);
    connect(ui->hashtagLineEdit, &QLineEdit::textChanged, this, &MastodonSettingWindow::hashtagChanged);
    connect(ui->hashtagLocalLineEdit, &QLineEdit::textChanged, this, &MastodonSettingWindow::hashtagLocalChanged);
    connect(ui->okButton, &QAbstractButton::clicked, this, &MastodonSettingWindow::okClicked);
    connect(ui->cancelButton, &QAbstractButton::clicked, this, &MastodonSettingWindow::cancelClicked);
    connect(ui->listComboBox, &QComboBox::currentIndexChanged, this, &MastodonSettingWindow::listSelected);

    updateUiFromSettings();
}

MastodonSettingWindow::~MastodonSettingWindow()
{
    delete ui;
}

void MastodonSettingWindow::updateUiFromSettings() {
    MastodonAccount *ma = this->account;
    ui->publicButton->setChecked(ma->source == MastodonAccount::Source::Public);
    ui->publicLocalButton->setChecked(ma->source == MastodonAccount::Source::PublicLocal);
    ui->publicRemoteButton->setChecked(ma->source == MastodonAccount::Source::PublicRemote);
    ui->hashtagButton->setChecked(ma->source == MastodonAccount::Source::Hashtag);
    ui->hashtagLocalButton->setChecked(ma->source == MastodonAccount::Source::HashtagLocal);
    ui->userButton->setChecked(ma->source == MastodonAccount::Source::User);
    ui->userNotificationButton->setChecked(ma->source == MastodonAccount::Source::UserNotification);
    ui->listButton->setChecked(ma->source == MastodonAccount::Source::List);
    ui->directButton->setChecked(ma->source == MastodonAccount::Source::Direct);

    currentHashtag = ma->hashtag;
    currentListId = ma->listId;
    updateButtonState();
    updateLists();
}

void MastodonSettingWindow::sourceButtonClicked() {
    updateButtonState();
}

void MastodonSettingWindow::hashtagChanged() {
    currentHashtag = ui->hashtagLineEdit->text();
    updateButtonState();
}

void MastodonSettingWindow::hashtagLocalChanged() {
    currentHashtag = ui->hashtagLocalLineEdit->text();
    updateButtonState();
}

void MastodonSettingWindow::listSelected(int index) {
    if (index < 0 || index >= this->lists.length()) return;
    MastodonList *list = this->lists.at(index);
    currentListId = list->id;
}

void MastodonSettingWindow::updateLists() {
    MastodonClient::shared().fetchLists(this->account->app, [=](QList<MastodonList*> lists) {
        this->lists = lists;
        for (auto l : lists) {
            l->setParent(this);
        }
        updateButtonState();
    });
}

void MastodonSettingWindow::updateButtonState() {
    ui->hashtagLineEdit->setVisible(ui->hashtagButton->isChecked());
    if (ui->hashtagLineEdit->text() != currentHashtag) {
        ui->hashtagLineEdit->setText(currentHashtag);
    }
    ui->hashtagLocalLineEdit->setVisible(ui->hashtagLocalButton->isChecked());
    if (ui->hashtagLocalLineEdit->text() != currentHashtag) {
        ui->hashtagLocalLineEdit->setText(currentHashtag);
    }
    ui->listComboBox->setVisible(ui->listButton->isChecked());
    bool isOkEnabled = true;
    if (ui->hashtagButton->isChecked() && currentHashtag.isEmpty()) {
        isOkEnabled = false;
    }
    if (ui->hashtagLocalButton->isChecked() && currentHashtag.isEmpty()) {
        isOkEnabled = false;
    }
    if (ui->listButton->isChecked()) {
        ui->listComboBox->clear();
        int i = 0;
        int selectedIndex = -1;
        for (auto list : this->lists) {
            ui->listComboBox->addItem(list->title, list->id);
            if (list->id == currentListId) {
                selectedIndex = i;
            }
            i++;
        }
        if (selectedIndex >= 0 && !currentListId.isEmpty()) {
            ui->listComboBox->setCurrentIndex(selectedIndex);
        } else {
            isOkEnabled = false;
        }
    }

    ui->okButton->setEnabled(isOkEnabled);
}

void MastodonSettingWindow::okClicked() {
    if (ui->publicButton->isChecked()) {
        this->account->source = MastodonAccount::Source::Public;

    } else if (ui->publicLocalButton->isChecked()) {
        this->account->source = MastodonAccount::Source::PublicLocal;

    } else if (ui->publicRemoteButton->isChecked()) {
        this->account->source = MastodonAccount::Source::PublicRemote;

    } else if (ui->hashtagButton->isChecked() && !currentHashtag.isEmpty()) {
        this->account->source = MastodonAccount::Source::Hashtag;
        this->account->hashtag = currentHashtag;

    } else if (ui->hashtagLocalButton->isChecked() && !currentHashtag.isEmpty()) {
        this->account->source = MastodonAccount::Source::HashtagLocal;
        this->account->hashtag = currentHashtag;

    } else if (ui->userButton->isChecked()) {
        this->account->source = MastodonAccount::Source::User;

    } else if (ui->userNotificationButton->isChecked()) {
        this->account->source = MastodonAccount::Source::UserNotification;

    } else if (ui->listButton->isChecked() && !currentListId.isEmpty()) {
        this->account->source = MastodonAccount::Source::List;
        this->account->listId = currentListId;

    } else if (ui->directButton->isChecked()) {
        this->account->source = MastodonAccount::Source::Direct;
    }

    accountUpdated(this->account);
    close();
}

void MastodonSettingWindow::cancelClicked() {
    close();
}
