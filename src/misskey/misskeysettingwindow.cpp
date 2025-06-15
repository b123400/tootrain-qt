#include "misskeysettingwindow.h"
#include "ui_misskeysettingwindow.h"
#include <QListWidgetItem>
#include "./misskeyclient.h"

MisskeySettingWindow::MisskeySettingWindow(MisskeyAccount *account, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MisskeySettingWindow)
{
    ui->setupUi(this);
    setFixedSize(size());
    this->account = account;
    account->setParent(this);
    this->allSources = MisskeyStreamSource::defaultSources(this);

    reloadListItems();
    MisskeyClient::shared().fetchAntennas(this->account, [=](QList<MisskeyStreamSource*> sources) {
        for (auto s : sources) {
            s->setParent(this);
        }
        this->allSources += sources;
        this->reloadListItems();
    });
    MisskeyClient::shared().fetchUserList(this->account, [=](QList<MisskeyStreamSource*> sources) {
        for (auto s : sources) {
            s->setParent(this);
        }
        this->allSources += sources;
        this->reloadListItems();
    });
    MisskeyClient::shared().fetchChannels(this->account, [=](QList<MisskeyStreamSource*> sources) {
        for (auto s : sources) {
            s->setParent(this);
        }
        this->allSources += sources;
        this->reloadListItems();
    });

    connect(ui->cancelButton, &QAbstractButton::clicked, this, &QWidget::close);
    connect(ui->okButton, &QAbstractButton::clicked, this, &MisskeySettingWindow::acceptClicked);
    connect(ui->listWidget, &QListWidget::itemChanged, this, &MisskeySettingWindow::listItemChanged);
}

MisskeySettingWindow::~MisskeySettingWindow()
{
    delete ui;
}

void MisskeySettingWindow::reloadListItems() {
    disconnect(ui->listWidget, &QListWidget::itemChanged, this, &MisskeySettingWindow::listItemChanged);
    ui->listWidget->clear();
    for (auto source : this->allSources) {
        auto item = new QListWidgetItem(source->displayName(), ui->listWidget);
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);

        bool isChecked = false;
        for (auto s : this->account->sources) {
            if (s->contentEqual(source)) {
                isChecked = true;
                break;
            }
        }
        if (isChecked) {
            item->setCheckState(Qt::Checked);
        } else {
            item->setCheckState(Qt::Unchecked);
        }
    }
    connect(ui->listWidget, &QListWidget::itemChanged, this, &MisskeySettingWindow::listItemChanged);
}

void MisskeySettingWindow::listItemChanged(QListWidgetItem *item) {
    auto index = ui->listWidget->indexFromItem(item);
    auto updatedSource = this->allSources[index.row()];
    bool isChecked = item->checkState() == Qt::Checked;

    int indexInAccount = -1;

    for (int i = 0; i < this->account->sources.length(); i++) {
        auto s = this->account->sources[i];
        if (s->contentEqual(updatedSource)) {
            indexInAccount = i;
            break;
        }
    }

    if (indexInAccount == -1 && isChecked) {
        this->account->sources.append(updatedSource->copy(this->account));
    } else if (indexInAccount >= 0 && !isChecked) {
        auto selectedSource = this->account->sources[indexInAccount];
        this->account->sources.removeAt(indexInAccount);
        delete selectedSource;
    }
}

void MisskeySettingWindow::acceptClicked() {
    emit accountUpdated(this->account);
    accept();
}
