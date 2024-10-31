#include "misskeysettingwindow.h"
#include "ui_misskeysettingwindow.h"
#include <QListWidgetItem>

MisskeySettingWindow::MisskeySettingWindow(MisskeyAccount *account, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MisskeySettingWindow)
{
    ui->setupUi(this);
    setFixedSize(size());
    this->account = account;
    this->allSources = MisskeyStreamSource::defaultSources(this);

    reloadListItems();

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &MisskeySettingWindow::acceptClicked);
    connect(ui->listWidget, &QListWidget::itemChanged, this, &MisskeySettingWindow::listItemChanged);
}

MisskeySettingWindow::~MisskeySettingWindow()
{
    delete ui;
}

void MisskeySettingWindow::reloadListItems() {
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
}

void MisskeySettingWindow::listItemChanged(QListWidgetItem *item) {
    auto index = ui->listWidget->indexFromItem(item);
    auto updatedSource = this->allSources[index.row()];
    bool isChecked = item->checkState() == Qt::Checked;
    qDebug() << "item changed: " << index.row() << "," << isChecked;

    int indexInAccount = -1;

    for (int i = 0; i < this->account->sources.length(); i++) {
        auto s = this->account->sources[i];
        if (s->contentEqual(updatedSource)) {
            indexInAccount = i;
            break;
        }
    }

    if (indexInAccount == -1 && item->checkState() == Qt::Checked) {
        qDebug() << "item checked";
        this->account->sources.append(updatedSource->copy(this->account));
    } else if (indexInAccount >= 0 && item->checkState() == Qt::Unchecked) {
        qDebug() << "item unchecked";
        auto selectedSource = this->account->sources[indexInAccount];
        this->account->sources.removeAt(indexInAccount);
        delete selectedSource;
    }
}

void MisskeySettingWindow::acceptClicked() {
    emit accountUpdated(this->account);
    accept();
}
