#include "mainwindow.h"
#include <QMenuBar>

#include "settingwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QMenuBar *menubar = this->menuBar();
    QMenu *fileMenu = menubar->addMenu("File");
    QAction *preferenceAction = fileMenu->addAction("Preferences");
    preferenceAction->setMenuRole(QAction::PreferencesRole);
    connect(preferenceAction, &QAction::triggered, this, &MainWindow::preferencesTriggered);
}

MainWindow::~MainWindow() {}

void MainWindow::preferencesTriggered(bool checked) {
    if (!this->settingWindow) {
        this->settingWindow = new SettingWindow();
    }
    this->settingWindow->show();
}
